/* obs-collection.c
 *
 * Copyright 2022 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "obs-collection.h"

struct _ObsCollection {
	GObject parent_instance;

	char *id;
	char *name;

	config_t *config;
};

G_DEFINE_FINAL_TYPE(ObsCollection, obs_collection, G_TYPE_OBJECT)

enum {
	PROP_0,
	PROP_CONFIG,
	PROP_ID,
	PROP_NAME,
	N_PROPS,
};

static GParamSpec *properties[N_PROPS];

static void obs_collection_finalize(GObject *object)
{
	ObsCollection *self = (ObsCollection *)object;

	g_clear_pointer(&self->id, g_free);
	g_clear_pointer(&self->name, g_free);

	G_OBJECT_CLASS(obs_collection_parent_class)->finalize(object);
}

static void obs_collection_get_property(GObject *object, guint prop_id,
					GValue *value, GParamSpec *pspec)
{
	ObsCollection *self = OBS_COLLECTION(object);

	switch (prop_id) {
	case PROP_ID:
		g_value_set_string(value, self->id);
		break;
	case PROP_NAME:
		g_value_set_string(value, self->name);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void obs_collection_set_property(GObject *object, guint prop_id,
					const GValue *value, GParamSpec *pspec)
{
	ObsCollection *self = OBS_COLLECTION(object);

	switch (prop_id) {
	case PROP_CONFIG:
		g_assert(self->config == NULL);
		self->config = g_value_get_pointer(value);

		self->name = g_strdup(
			config_get_string(self->config, "general", "name"));
		break;
	case PROP_ID:
		g_assert(self->id == NULL);
		self->id = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void obs_collection_class_init(ObsCollectionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->finalize = obs_collection_finalize;
	object_class->get_property = obs_collection_get_property;
	object_class->set_property = obs_collection_set_property;

	properties[PROP_CONFIG] = g_param_spec_pointer(
		"config", NULL, NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
			G_PARAM_STATIC_STRINGS);

	properties[PROP_ID] =
		g_param_spec_string("id", NULL, NULL, NULL,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
					    G_PARAM_STATIC_STRINGS);

	properties[PROP_NAME] =
		g_param_spec_string("name", NULL, NULL, NULL,
				    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void obs_collection_init(ObsCollection *self) {}

ObsCollection *obs_collection_new(const char *id, config_t *config)
{
	return g_object_new(OBS_TYPE_COLLECTION, "id", id, "config", config,
			    NULL);
}

const char *obs_collection_get_id(ObsCollection *self)
{
	g_return_val_if_fail(OBS_IS_COLLECTION(self), NULL);

	return self->id;
}

void obs_collection_set_name(ObsCollection *self, const char *name)
{
	g_return_if_fail(OBS_IS_COLLECTION(self));
	g_return_if_fail(g_utf8_validate(name, -1, NULL));
	g_return_if_fail(g_utf8_strlen(name, -1) > 0 && *name != '\n');

	if (g_strcmp0(self->name, name) == 0)
		return;

	g_clear_pointer(&self->name, g_free);
	self->name = g_strdup(name);

	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_NAME]);
}

const char *obs_collection_get_name(ObsCollection *self)
{
	g_return_val_if_fail(OBS_IS_COLLECTION(self), NULL);

	return self->name;
}
