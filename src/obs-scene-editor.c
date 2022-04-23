/* obs-scene-editor.c
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

#include "obs-scene-editor.h"

#include "obs-display-widget.h"

#include <obs.h>

struct _ObsSceneEditor
{
	GtkWidget parent_instance;

	GtkWidget *main_box;
	ObsDisplayWidget *display_widget;
};

G_DEFINE_FINAL_TYPE (ObsSceneEditor, obs_scene_editor, GTK_TYPE_WIDGET)

static void
draw_cb(gpointer user_data,
        uint32_t width,
        uint32_t height)
{
	obs_render_main_texture();
}

static void
obs_scene_editor_dispose (GObject *object)
{
	ObsSceneEditor *self = OBS_SCENE_EDITOR(object);

	g_clear_pointer(&self->main_box, gtk_widget_unparent);

	G_OBJECT_CLASS(obs_scene_editor_parent_class)->dispose(object);
}

static void
obs_scene_editor_class_init (ObsSceneEditorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	g_type_ensure(OBS_TYPE_DISPLAY_WIDGET);

	object_class->dispose = obs_scene_editor_dispose;

	gtk_widget_class_set_template_from_resource (widget_class, "/com/obsproject/Studio/GTK4/ui/obs-scene-editor.ui");

	gtk_widget_class_bind_template_child (widget_class, ObsSceneEditor, display_widget);
	gtk_widget_class_bind_template_child (widget_class, ObsSceneEditor, main_box);

	gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
	gtk_widget_class_set_css_name(widget_class, "sceneeditor");
}

static void
obs_scene_editor_init (ObsSceneEditor *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));

	obs_display_widget_add_draw_callback(self->display_widget, draw_cb, self);
}
