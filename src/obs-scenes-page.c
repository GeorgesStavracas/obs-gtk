/* obs-scenes-page.c
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

#include "obs-scenes-page.h"

#include "scenes/obs-new-collection-dialog.h"
#include "obs-scene-editor.h"

struct _ObsScenesPage {
	AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE(ObsScenesPage, obs_scenes_page, ADW_TYPE_BIN)

static void on_new_collection_action_cb(GtkWidget *widget,
					const char *action_name,
					GVariant *parameter)
{
	GtkWindow *dialog;

	g_assert(g_strcmp0(action_name, "scenes.new-collection") == 0);
	g_assert(parameter == NULL);

	dialog = obs_new_collection_dialog_new();
	gtk_window_set_transient_for(dialog,
				     GTK_WINDOW(gtk_widget_get_root(widget)));
	gtk_window_present(dialog);
}

static void obs_scenes_page_class_init(ObsScenesPageClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	g_type_ensure(OBS_TYPE_SCENE_EDITOR);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/com/obsproject/Studio/GTK4/ui/obs-scenes-page.ui");

	gtk_widget_class_install_action(widget_class, "scenes.new-collection",
					NULL, on_new_collection_action_cb);
}

static void obs_scenes_page_init(ObsScenesPage *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
}
