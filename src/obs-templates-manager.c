/* obs-templates-manager.c
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

#include "obs-templates-manager.h"

#include "obs-collection-template.h"

#include <glib/gi18n.h>

struct _ObsTemplatesManager {
	GObject parent_instance;

	GListStore *collection_templates;

	gboolean loaded;
};

G_DEFINE_FINAL_TYPE(ObsTemplatesManager, obs_templates_manager, G_TYPE_OBJECT)

static const struct {
	const char *id;
	const char *name;
} collection_templates[] = {
	{
		"empty",
		N_("Empty"),
	},
};

static void load_collection_templates(ObsTemplatesManager *self)
{
	for (size_t i = 0; i < G_N_ELEMENTS(collection_templates); i++) {
		ObsCollectionTemplate *collection_template;
		GError *error = NULL;

		collection_template = obs_collection_template_new(
			collection_templates[i].id,
			collection_templates[i].name, &error);
		if (error) {
			g_warning("Failed to load template '%s': %s",
				  collection_templates[i].id, error->message);
			g_clear_error(&error);
			continue;
		}

		g_list_store_append(self->collection_templates,
				    collection_template);
		g_object_unref(collection_template);
	}
}

static void obs_templates_manager_finalize(GObject *object)
{
	ObsTemplatesManager *self = (ObsTemplatesManager *)object;

	g_clear_object(&self->collection_templates);

	G_OBJECT_CLASS(obs_templates_manager_parent_class)->finalize(object);
}

static void obs_templates_manager_class_init(ObsTemplatesManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->finalize = obs_templates_manager_finalize;
}

static void obs_templates_manager_init(ObsTemplatesManager *self)
{
	self->collection_templates =
		g_list_store_new(OBS_TYPE_COLLECTION_TEMPLATE);
}

ObsTemplatesManager *obs_templates_manager_new(void)
{
	return g_object_new(OBS_TYPE_TEMPLATES_MANAGER, NULL);
}

void obs_templates_manager_load_templates(ObsTemplatesManager *self)
{
	g_return_if_fail(OBS_IS_TEMPLATES_MANAGER(self));
	g_return_if_fail(!self->loaded);

	load_collection_templates(self);

	self->loaded = TRUE;
}

GListModel *
obs_templates_manager_get_collection_templates(ObsTemplatesManager *self)
{
	g_return_val_if_fail(OBS_IS_TEMPLATES_MANAGER(self), NULL);
	g_return_val_if_fail(self->loaded, NULL);

	return G_LIST_MODEL(self->collection_templates);
}
