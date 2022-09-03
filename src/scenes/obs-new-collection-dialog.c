/* obs-new-collection-dialog.c
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

#include "obs-new-collection-dialog.h"

#include <glib/gi18n.h>

#include "obs-application.h"
#include "obs-collection-template.h"
#include "obs-collection-template-icon.h"
#include "obs-templates-manager.h"

struct _ObsNewCollectionDialog {
	AdwWindow parent_instance;

	GtkEditable *collection_name_row;
	GtkFlowBox *templates_flowbox;
};

G_DEFINE_FINAL_TYPE(ObsNewCollectionDialog, obs_new_collection_dialog,
		    ADW_TYPE_WINDOW)

static void validate_dialog(ObsNewCollectionDialog *self)
{
	GList *selected_template = NULL;
	const char *text;

	selected_template =
		gtk_flow_box_get_selected_children(self->templates_flowbox);
	text = gtk_editable_get_text(self->collection_name_row);

	gtk_widget_action_set_enabled(
		GTK_WIDGET(self), "new-collection-dialog.create-collection",
		selected_template != NULL && text != NULL && *text != '\0' &&
			g_utf8_validate(text, -1, NULL));

	g_list_free(selected_template);
}

static GtkWidget *create_template_icon_func(gpointer item, gpointer user_data)
{
	return obs_collection_template_icon_new(item);
}

static void on_collection_created_cb(GObject *source_object,
				     GAsyncResult *result, gpointer user_data)
{
	ObsNewCollectionDialog *self = OBS_NEW_COLLECTION_DIALOG(user_data);
	GError *error = NULL;

	obs_collection_template_create_finish(
		OBS_COLLECTION_TEMPLATE(source_object), result, &error);

	if (error) {
		g_warning("Failed to create collection: %s", error->message);
		g_clear_error(&error);
	}

	gtk_window_close(GTK_WINDOW(self));
}

static void on_create_collection_action_activated_cb(GtkWidget *widget,
						     const char *action_name,
						     GVariant *parameters)
{
	ObsCollectionTemplateIcon *icon;
	ObsNewCollectionDialog *self;
	ObsCollectionTemplate *template;
	GList *selected_template;
	const char *collection_name;

	self = OBS_NEW_COLLECTION_DIALOG(widget);

	g_assert(g_strcmp0(action_name,
			   "new-collection-dialog.create-collection") == 0);
	g_assert(parameters == NULL);

	selected_template =
		gtk_flow_box_get_selected_children(self->templates_flowbox);
	g_assert(selected_template != NULL);

	icon = OBS_COLLECTION_TEMPLATE_ICON(selected_template->data);
	collection_name = gtk_editable_get_text(self->collection_name_row);

	template = obs_collection_template_icon_get_collection_template(icon);
	obs_collection_template_create_async(template, collection_name, NULL,
					     on_collection_created_cb, self);

	g_list_free(selected_template);
}

static void
on_collection_name_row_text_changed_cb(GtkEditable *collection_name_row,
				       GParamSpec *pspec,
				       ObsNewCollectionDialog *self)
{
	validate_dialog(self);
}

static void
on_templates_flowbox_selected_children_changed_cb(GtkFlowBox *templates_flowbox,
						  ObsNewCollectionDialog *self)
{
	validate_dialog(self);
}

static void
obs_new_collection_dialog_class_init(ObsNewCollectionDialogClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/com/obsproject/Studio/GTK4/ui/scenes/obs-new-collection-dialog.ui");

	gtk_widget_class_bind_template_child(
		widget_class, ObsNewCollectionDialog, collection_name_row);
	gtk_widget_class_bind_template_child(
		widget_class, ObsNewCollectionDialog, templates_flowbox);

	gtk_widget_class_bind_template_callback(
		widget_class, on_collection_name_row_text_changed_cb);
	gtk_widget_class_bind_template_callback(
		widget_class,
		on_templates_flowbox_selected_children_changed_cb);

	gtk_widget_class_install_action(
		widget_class, "new-collection-dialog.create-collection", NULL,
		on_create_collection_action_activated_cb);
}

static void obs_new_collection_dialog_init(ObsNewCollectionDialog *self)
{
	ObsTemplatesManager *templates_manager;
	GApplication *application;
	GListModel *collection_templates;

	gtk_widget_init_template(GTK_WIDGET(self));

	application = g_application_get_default();
	templates_manager = obs_application_get_templates_manager(
		OBS_APPLICATION(application));
	collection_templates = obs_templates_manager_get_collection_templates(
		templates_manager);

	gtk_flow_box_bind_model(self->templates_flowbox, collection_templates,
				create_template_icon_func, self, NULL);

	gtk_widget_action_set_enabled(GTK_WIDGET(self),
				      "new-collection-dialog.create-collection",
				      FALSE);
}

GtkWindow *obs_new_collection_dialog_new(void)
{
	return g_object_new(OBS_TYPE_NEW_COLLECTION_DIALOG, NULL);
}
