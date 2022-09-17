/* obs-collection-template.c
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

#include "obs-collection-template.h"

#include "obs-application.h"
#include "obs-config-manager.h"

#include <util/config-file.h>

#define RESOURCE_PATH \
	"/com/obsproject/Studio/GTK4/assets/collection-templates/%s.svg"

typedef struct {
	char *collection_name;
	config_t *config;
} CreateData;

struct _ObsCollectionTemplate {
	GObject parent_instance;

	char *id;
	char *name;
	GdkPaintable *icon;
};

static void obs_collection_template_initable_iface_init(GInitableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE(
	ObsCollectionTemplate, obs_collection_template, G_TYPE_OBJECT,
	G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,
			      obs_collection_template_initable_iface_init))

enum {
	PROP_0,
	PROP_ICON,
	PROP_ID,
	PROP_NAME,
	N_PROPS,
};

static GParamSpec *properties[N_PROPS];

static void create_data_free(gpointer data)
{
	CreateData *create_data = data;

	if (!data)
		return;

	g_clear_pointer(&create_data->collection_name, g_free);
	g_free(create_data);
}

static void create_collection_in_thread_func(GTask *task,
					     gpointer source_object,
					     gpointer task_data,
					     GCancellable *cancellable)
{
	// TODO

	g_task_return_pointer(task, NULL, NULL);
}

GdkPaintable *create_paintable_from_resource(const char *resource_path)
{
	GdkPaintable *paintable;
	GtkWidget *picture;

	picture = gtk_picture_new_for_resource(resource_path);
	g_object_ref_sink(picture);

	paintable = gtk_picture_get_paintable(GTK_PICTURE(picture));
	g_object_ref(paintable);

	g_object_unref(picture);

	return paintable;
}

static gboolean obs_collection_template_initable_init(GInitable *initable,
						      GCancellable *cancellable,
						      GError **error)
{
	ObsCollectionTemplate *self = OBS_COLLECTION_TEMPLATE(initable);
	char path[256];

	g_snprintf(path, sizeof(path), RESOURCE_PATH, self->id);
	self->icon = create_paintable_from_resource(path);

	return TRUE;
}

static void obs_collection_template_initable_iface_init(GInitableIface *iface)
{
	iface->init = obs_collection_template_initable_init;
}

static void obs_collection_template_finalize(GObject *object)
{
	ObsCollectionTemplate *self = (ObsCollectionTemplate *)object;

	g_clear_object(&self->icon);
	g_clear_pointer(&self->id, g_free);
	g_clear_pointer(&self->name, g_free);

	G_OBJECT_CLASS(obs_collection_template_parent_class)->finalize(object);
}

static void obs_collection_template_get_property(GObject *object, guint prop_id,
						 GValue *value,
						 GParamSpec *pspec)
{
	ObsCollectionTemplate *self = OBS_COLLECTION_TEMPLATE(object);

	switch (prop_id) {
	case PROP_ICON:
		g_value_set_object(value, self->icon);
		break;
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

static void obs_collection_template_set_property(GObject *object, guint prop_id,
						 const GValue *value,
						 GParamSpec *pspec)
{
	ObsCollectionTemplate *self = OBS_COLLECTION_TEMPLATE(object);

	switch (prop_id) {
	case PROP_ID:
		g_assert(self->id == NULL);
		self->id = g_value_dup_string(value);
		break;
	case PROP_NAME:
		g_assert(self->name == NULL);
		self->name = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
obs_collection_template_class_init(ObsCollectionTemplateClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->finalize = obs_collection_template_finalize;
	object_class->get_property = obs_collection_template_get_property;
	object_class->set_property = obs_collection_template_set_property;

	properties[PROP_ICON] =
		g_param_spec_object("icon", NULL, NULL, GDK_TYPE_PAINTABLE,
				    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	properties[PROP_ID] =
		g_param_spec_string("id", NULL, NULL, NULL,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
					    G_PARAM_STATIC_STRINGS);

	properties[PROP_NAME] =
		g_param_spec_string("name", NULL, NULL, NULL,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
					    G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void obs_collection_template_init(ObsCollectionTemplate *self) {}

ObsCollectionTemplate *
obs_collection_template_new(const char *id, const char *name, GError **error)
{
	return g_initable_new(OBS_TYPE_COLLECTION_TEMPLATE, NULL, error, "id",
			      id, "name", name, NULL);
}

const char *obs_collection_template_get_id(ObsCollectionTemplate *self)
{
	g_return_val_if_fail(OBS_IS_COLLECTION_TEMPLATE(self), NULL);
	return self->id;
}

const char *obs_collection_template_get_name(ObsCollectionTemplate *self)
{
	g_return_val_if_fail(OBS_IS_COLLECTION_TEMPLATE(self), NULL);
	return self->name;
}

GdkPaintable *obs_collection_template_get_icon(ObsCollectionTemplate *self)
{
	g_return_val_if_fail(OBS_IS_COLLECTION_TEMPLATE(self), NULL);
	return self->icon;
}

void obs_collection_template_create_async(ObsCollectionTemplate *self,
					  const char *collection_name,
					  GCancellable *cancellable,
					  GAsyncReadyCallback callback,
					  gpointer user_data)
{
	ObsConfigManager *config_manager;
	GApplication *application;
	CreateData *data = NULL;
	config_t *config;
	GTask *task = NULL;

	g_return_if_fail(OBS_IS_COLLECTION_TEMPLATE(self));
	g_return_if_fail(!cancellable || G_IS_CANCELLABLE(cancellable));
	g_return_if_fail(collection_name &&
			 g_utf8_validate(collection_name, -1, NULL));

	application = g_application_get_default();
	config_manager = obs_application_get_config_manager(
		OBS_APPLICATION(application));

	config = obs_config_manager_open(config_manager,
					 OBS_CONFIG_SCOPE_SCENE_COLLECTION,
					 collection_name, CONFIG_OPEN_EXISTING);
	if (config) {
		obs_config_manager_release(config_manager,
					   OBS_CONFIG_SCOPE_SCENE_COLLECTION,
					   collection_name);

		task = g_task_new(self, cancellable, callback, user_data);
		g_task_set_source_tag(task,
				      obs_collection_template_create_async);
		g_task_return_new_error(task, G_IO_ERROR, G_IO_ERROR_EXISTS,
					"Collection already exists");
		g_object_unref(task);
		return;
	}

	data = g_new0(CreateData, 1);
	data->collection_name = g_strdup(collection_name);
	data->config = obs_config_manager_open(
		config_manager, OBS_CONFIG_SCOPE_SCENE_COLLECTION,
		collection_name, CONFIG_OPEN_ALWAYS);

	task = g_task_new(self, cancellable, callback, user_data);
	g_task_set_task_data(task, data, create_data_free);
	g_task_set_source_tag(task, obs_collection_template_create_async);
	g_task_run_in_thread(task, create_collection_in_thread_func);

	g_object_unref(task);
}

gpointer obs_collection_template_create_finish(ObsCollectionTemplate *self,
					       GAsyncResult *result,
					       GError **error)
{
	g_return_val_if_fail(OBS_IS_COLLECTION_TEMPLATE(self), NULL);
	g_return_val_if_fail(g_task_is_valid(result, self), NULL);

	return g_task_propagate_pointer(G_TASK(result), error);
}
