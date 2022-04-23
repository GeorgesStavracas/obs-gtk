/* obs-display-widget.c
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

#include "obs-display-widget.h"

struct _ObsDisplayWidget
{
	GtkWidget parent_instance;

	GtkWidget *picture;

	ObsDisplayRenderer *display_renderer;
};

G_DEFINE_FINAL_TYPE (ObsDisplayWidget, obs_display_widget, GTK_TYPE_WIDGET)

static void
on_display_renderer_notify_cb(GtkMediaStream *stream,
                              GParamSpec     *pspec,
                              GtkVideo       *self)
{
	if (gtk_media_stream_is_prepared(stream) &&
	    gtk_widget_get_mapped(GTK_WIDGET (self)))
		gtk_media_stream_play(stream);
}

static void
obs_display_widget_realize(GtkWidget *widget)
{
	ObsDisplayWidget *self = OBS_DISPLAY_WIDGET(widget);
	GdkSurface *surface;

	GTK_WIDGET_CLASS(obs_display_widget_parent_class)->realize(widget);

	surface = gtk_native_get_surface(gtk_widget_get_native (widget));
	gtk_media_stream_realize(GTK_MEDIA_STREAM(self->display_renderer), surface);
}

static void
obs_display_widget_unrealize(GtkWidget *widget)
{
	ObsDisplayWidget *self = OBS_DISPLAY_WIDGET(widget);
	GdkSurface *surface;

	surface = gtk_native_get_surface (gtk_widget_get_native (widget));

	gtk_media_stream_pause(GTK_MEDIA_STREAM(self->display_renderer));
	gtk_media_stream_unrealize(GTK_MEDIA_STREAM(self->display_renderer), surface);

	GTK_WIDGET_CLASS(obs_display_widget_parent_class)->unrealize(widget);
}

static void
obs_display_widget_map(GtkWidget *widget)
{
	ObsDisplayWidget *self = OBS_DISPLAY_WIDGET(widget);

	GTK_WIDGET_CLASS(obs_display_widget_parent_class)->map(widget);

	if (gtk_media_stream_is_prepared(GTK_MEDIA_STREAM(self->display_renderer)))
		gtk_media_stream_play(GTK_MEDIA_STREAM(self->display_renderer));
}

static void
obs_display_widget_hide(GtkWidget *widget)
{
	ObsDisplayWidget *self = OBS_DISPLAY_WIDGET(widget);

	gtk_media_stream_pause(GTK_MEDIA_STREAM(self->display_renderer));

	GTK_WIDGET_CLASS(obs_display_widget_parent_class)->hide(widget);
}

static void
obs_display_widget_dispose(GObject *object)
{
	ObsDisplayWidget *self = (ObsDisplayWidget *)object;

	g_clear_pointer(&self->picture, gtk_widget_unparent);
	g_clear_object(&self->display_renderer);

	G_OBJECT_CLASS (obs_display_widget_parent_class)->dispose (object);
}

static void
obs_display_widget_class_init(ObsDisplayWidgetClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	object_class->dispose = obs_display_widget_dispose;

	widget_class->realize = obs_display_widget_realize;
	widget_class->unrealize = obs_display_widget_unrealize;
	widget_class->map = obs_display_widget_map;
	widget_class->hide = obs_display_widget_hide;

	gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);
	gtk_widget_class_set_css_name(widget_class, "displaywidget");
}

static void
obs_display_widget_init(ObsDisplayWidget *self)
{
	self->picture = gtk_picture_new();
	gtk_widget_set_parent(self->picture, GTK_WIDGET(self));

	self->display_renderer = obs_display_renderer_new();
	g_signal_connect(self->display_renderer, "notify::prepared", G_CALLBACK(on_display_renderer_notify_cb), self);
	gtk_picture_set_paintable(GTK_PICTURE(self->picture),
				  GDK_PAINTABLE(self->display_renderer));
}

ObsDisplayWidget *
obs_display_widget_new(void)
{
	return g_object_new(OBS_TYPE_DISPLAY_WIDGET, NULL);
}

void
obs_display_widget_add_draw_callback(ObsDisplayWidget   *self,
                                     ObsDisplayDrawFunc  draw_func,
                                     gpointer            user_data)
{
	g_return_if_fail(OBS_IS_DISPLAY_WIDGET (self));

	obs_display_renderer_add_draw_callback(self->display_renderer,
					       draw_func,
					       user_data);
}

void
obs_display_widget_remove_draw_callback(ObsDisplayWidget   *self,
                                        ObsDisplayDrawFunc  draw_func,
                                        gpointer            user_data)
{
	g_return_if_fail(OBS_IS_DISPLAY_WIDGET (self));

	obs_display_renderer_remove_draw_callback(self->display_renderer,
						  draw_func,
						  user_data);
}
