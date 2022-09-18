/* obs-window.c
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

#include "obs-window.h"

#include "obs-activities-page.h"
#include "obs-mixer-page.h"
#include "obs-collections-page.h"
#include "preferences/obs-preferences-dialog.h"
#include "profiles/obs-profiles-dialog.h"

#include <obs.h>
#include <util/platform.h>
#include <util/threading.h>

struct _ObsWindow {
	AdwApplicationWindow parent_instance;

	GtkLabel *fps_label;

	guint fps_timeout_id;
};

G_DEFINE_FINAL_TYPE(ObsWindow, obs_window, ADW_TYPE_APPLICATION_WINDOW)

static int reset_video(void)
{
	struct obs_video_info ovi = {
		.graphics_module = DL_OPENGL,
		.fps_num = 60,
		.fps_den = 1,
		.base_width = 1920,
		.base_height = 1080,
		.output_width = 1920,
		.output_height = 1080,
		.output_format = VIDEO_FORMAT_NV12,
		.colorspace = VIDEO_CS_709,
		.range = VIDEO_RANGE_PARTIAL,
		.scale_type = OBS_SCALE_BILINEAR,
		.gpu_conversion = true,
	};

	return obs_reset_video(&ovi);
}

static void update_status_bar_info(ObsWindow *self)
{
	char *fps_string;

	fps_string = g_strdup_printf("%.2f FPS", obs_get_active_fps());
	gtk_label_set_label(self->fps_label, fps_string);
	g_free(fps_string);
}

static gboolean update_fps_cb(gpointer user_data)
{
	ObsWindow *self = OBS_WINDOW(user_data);

	update_status_bar_info(self);

	return G_SOURCE_CONTINUE;
}

/*
 * Actions
 */

static void on_open_url_action_cb(GtkWidget *widget, const char *action_name,
				  GVariant *parameter)
{
	const char *url;

	g_assert(g_strcmp0(action_name, "window.open-url") == 0);
	g_assert(parameter != NULL &&
		 g_variant_check_format_string(parameter, "s", FALSE));

	url = g_variant_get_string(parameter, NULL);
	gtk_show_uri(GTK_WINDOW(widget), url, GDK_CURRENT_TIME);
}

static void on_show_preferences_action_cb(GtkWidget *widget,
					  const char *action_name,
					  GVariant *parameter)
{
	GtkWidget *dialog;

	g_assert(g_strcmp0(action_name, "window.show-preferences") == 0);
	g_assert(parameter == NULL);

	dialog = obs_preferences_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(widget));
	gtk_window_present(GTK_WINDOW(dialog));
}

static void on_show_profiles_action_cb(GtkWidget *widget,
				       const char *action_name,
				       GVariant *parameter)
{
	GtkWindow *dialog;

	g_assert(g_strcmp0(action_name, "window.show-profiles") == 0);
	g_assert(parameter == NULL);

	dialog = obs_profiles_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(widget));
	gtk_window_present(GTK_WINDOW(dialog));
}

/*
 * GObject overrides
 */

static void obs_window_finalize(GObject *object)
{
	ObsWindow *self = (ObsWindow *)object;

	g_clear_handle_id(&self->fps_timeout_id, g_source_remove);

	G_OBJECT_CLASS(obs_window_parent_class)->finalize(object);
}

static void obs_window_constructed(GObject *object)
{
	ObsWindow *self = (ObsWindow *)object;

	G_OBJECT_CLASS(obs_window_parent_class)->constructed(object);

	self->fps_timeout_id = g_timeout_add_seconds_full(
		G_PRIORITY_LOW, 2, update_fps_cb, self, NULL);
	update_status_bar_info(self);

	int status = reset_video();
	if (status != VIDEO_OUTPUT_SUCCESS)
		g_warning("Error: %d", status);
}

static void obs_window_class_init(ObsWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	g_type_ensure(OBS_TYPE_ACTIVITIES_PAGE);
	g_type_ensure(OBS_TYPE_MIXER_PAGE);
	g_type_ensure(OBS_TYPE_COLLECTIONS_PAGE);

	object_class->finalize = obs_window_finalize;
	object_class->constructed = obs_window_constructed;

	gtk_widget_class_set_template_from_resource(
		widget_class, "/com/obsproject/Studio/GTK4/ui/obs-window.ui");

	gtk_widget_class_bind_template_child(widget_class, ObsWindow,
					     fps_label);

	gtk_widget_class_install_action(widget_class, "window.open-url", "s",
					on_open_url_action_cb);
	gtk_widget_class_install_action(widget_class, "window.show-preferences",
					NULL, on_show_preferences_action_cb);
	gtk_widget_class_install_action(widget_class, "window.show-profiles",
					NULL, on_show_profiles_action_cb);
}

static void obs_window_init(ObsWindow *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
}

ObsWindow *obs_window_new(GApplication *application)
{
	return g_object_new(OBS_TYPE_WINDOW, "application", application, NULL);
}
