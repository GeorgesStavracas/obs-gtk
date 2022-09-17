/* obs-mixer-page.c
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

#include "obs-mixer-page.h"

#include "audio/obs-audio-device-controls.h"
#include "obs-application.h"
#include "obs-audio-controller.h"

struct _ObsMixerPage {
	AdwBin parent_instance;

	GtkNoSelection *no_selection_model;
};

G_DEFINE_FINAL_TYPE(ObsMixerPage, obs_mixer_page, ADW_TYPE_BIN)

static void obs_mixer_page_class_init(ObsMixerPageClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	g_type_ensure(OBS_TYPE_AUDIO_DEVICE_CONTROLS);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/com/obsproject/Studio/GTK4/ui/obs-mixer-page.ui");

	gtk_widget_class_bind_template_child(widget_class, ObsMixerPage,
					     no_selection_model);
}

static void obs_mixer_page_init(ObsMixerPage *self)
{
	ObsAudioController *audio_controller;
	GApplication *application;

	gtk_widget_init_template(GTK_WIDGET(self));

	application = g_application_get_default();
	audio_controller = obs_application_get_audio_controller(
		OBS_APPLICATION(application));
	gtk_no_selection_set_model(
		self->no_selection_model,
		obs_audio_controller_get_devices(audio_controller));
}
