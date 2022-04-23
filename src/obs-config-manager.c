/* obs-config-manager.c
 *
 * Copyright 2022 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "obs-config-manager.h"

#define N_SCOPES (OBS_CONFIG_SCOPE_PLUGIN + 1)

typedef struct {
	grefcount refcount;
	config_t *config;
} ConfigData;

struct _ObsConfigManager
{
	GObject parent_instance;

	GHashTable *configs[N_SCOPES];
};

G_DEFINE_FINAL_TYPE (ObsConfigManager, obs_config_manager, G_TYPE_OBJECT)

static void
config_data_free(gpointer data)
{
	ConfigData *config_data = data;

	if (!config_data)
		return;

	g_clear_pointer(&config_data->config, config_close);
	g_free(config_data);
}

static char *
build_path(ObsConfigScope  scope,
           const char     *id)
{
	const char *config_dir;
	char *filename = NULL;
	char *path;

	config_dir = g_get_user_config_dir();

	switch (scope) {
	case OBS_CONFIG_SCOPE_GLOBAL:
		filename = g_strdup_printf("%s.ini", id);
		path = g_build_filename(config_dir, filename, NULL);
		break;
	case OBS_CONFIG_SCOPE_PROFILE:
		path = g_build_filename(config_dir, "profiles", id, "config.ini", NULL);
		break;
	case OBS_CONFIG_SCOPE_SCENE_COLLECTION:
		path = g_build_filename(config_dir, "scenes", id, "config.ini", NULL);
		break;
	case OBS_CONFIG_SCOPE_PLUGIN:
		path = g_build_filename(config_dir, "plugins", id, "config.ini", NULL);
		break;
	default:
		g_assert_not_reached();
	}

	g_free(filename);

	return path;
}

static void
obs_config_manager_finalize (GObject *object)
{
	ObsConfigManager *self = (ObsConfigManager *)object;

	for (int i = 0; i < N_SCOPES; i++)
		g_clear_pointer(&self->configs[i], g_hash_table_destroy);

	G_OBJECT_CLASS (obs_config_manager_parent_class)->finalize (object);
}

static void
obs_config_manager_class_init (ObsConfigManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = obs_config_manager_finalize;
}

static void
obs_config_manager_init (ObsConfigManager *self)
{
	for (int i = 0; i < N_SCOPES; i++) {
		self->configs[i] = g_hash_table_new_full(g_str_hash, g_str_equal,
							 g_free, config_data_free);
	}
}

ObsConfigManager *
obs_config_manager_new (void)
{
	return g_object_new (OBS_TYPE_CONFIG_MANAGER, NULL);
}

config_t *
obs_config_manager_open (ObsConfigManager      *self,
                         ObsConfigScope         scope,
                         const char            *id,
                         enum config_open_type  open_type)
{
	ConfigData *data;

	g_return_val_if_fail(OBS_IS_CONFIG_MANAGER(self), NULL);
	g_return_val_if_fail(id != NULL, NULL);

	data = g_hash_table_lookup(self->configs[scope], id);

	if (!data) {
		config_t *config;
		char *config_file;

		config_file = build_path(scope, id);

		if (config_open(&config, config_file, open_type) != CONFIG_SUCCESS) {
			g_free(config_file);
			return NULL;
		}

		data = g_new0(ConfigData, 1);
		data->config = config;
		g_ref_count_init(&data->refcount);

		g_hash_table_insert (self->configs[scope], g_strdup(id), data);

		g_free(config_file);
	} else {
		g_ref_count_inc(&data->refcount);
	}

	return data->config;
}

void
obs_config_manager_release (ObsConfigManager *self,
                            ObsConfigScope    scope,
                            const char       *id)
{
	ConfigData *data;

	g_return_if_fail(OBS_IS_CONFIG_MANAGER(self));
	g_return_if_fail(id != NULL);

	data = g_hash_table_lookup(self->configs[scope], id);

	if (data && g_ref_count_dec(&data->refcount))
		g_hash_table_remove(self->configs[scope], id);
}
