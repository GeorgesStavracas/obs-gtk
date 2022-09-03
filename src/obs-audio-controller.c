/* obs-audio-controller.c
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

#include "obs-audio-controller.h"

#include "obs-audio-device.h"

#include <obs.h>
#include <gtk/gtk.h>

#ifdef __APPLE__
#define INPUT_AUDIO_SOURCE "coreaudio_input_capture"
#define OUTPUT_AUDIO_SOURCE "coreaudio_output_capture"
#elif _WIN32
#define INPUT_AUDIO_SOURCE "wasapi_input_capture"
#define OUTPUT_AUDIO_SOURCE "wasapi_output_capture"
#else
#define INPUT_AUDIO_SOURCE "pulse_input_capture"
#define OUTPUT_AUDIO_SOURCE "pulse_output_capture"
#endif

struct _ObsAudioController {
	GObject parent_instance;

	GListStore *input_devices;
	GListStore *output_devices;
	GListModel *all_devices;

	bool shutdown;
};

G_DEFINE_FINAL_TYPE(ObsAudioController, obs_audio_controller, G_TYPE_OBJECT)

static void populate_all_devices(ObsAudioController *self)
{
	obs_properties_t *output_props;
	obs_properties_t *input_props;

	/* Input */
	input_props = obs_get_source_properties(INPUT_AUDIO_SOURCE);
	if (input_props) {
		obs_property_t *inputs =
			obs_properties_get(input_props, "device_id");
		size_t n_inputs = obs_property_list_item_count(inputs);

		for (size_t i = 0; i < n_inputs; i++) {
			ObsAudioDevice *audio_device;
			obs_source_t *source;
			const char *input_string;
			const char *input_name;
			obs_data_t *settings;

			if (g_strcmp0(obs_property_list_item_string(inputs, i),
				      "default") == 0)
				continue;

			input_name = obs_property_list_item_name(inputs, i);
			input_string = obs_property_list_item_string(inputs, i);

			settings = obs_data_create();
			obs_data_set_string(settings, "device_id",
					    input_string);
			source = obs_source_create(INPUT_AUDIO_SOURCE,
						   input_name, settings, NULL);

			audio_device = obs_audio_device_new(
				OBS_AUDIO_DEVICE_INPUT, source);
			g_list_store_append(self->input_devices, audio_device);

			g_object_unref(audio_device);
		}
	}
	g_clear_pointer(&input_props, obs_properties_destroy);

	output_props = obs_get_source_properties(OUTPUT_AUDIO_SOURCE);
	if (output_props) {
		obs_property_t *outputs =
			obs_properties_get(output_props, "device_id");
		size_t n_outputs = obs_property_list_item_count(outputs);

		for (size_t i = 0; i < n_outputs; i++) {
			ObsAudioDevice *audio_device;
			obs_source_t *source;
			const char *output_string;
			const char *output_name;
			obs_data_t *settings;

			if (g_strcmp0(obs_property_list_item_string(outputs, i),
				      "default") == 0)
				continue;

			output_name = obs_property_list_item_name(outputs, i);
			output_string =
				obs_property_list_item_string(outputs, i);

			settings = obs_data_create();
			obs_data_set_string(settings, "device_id",
					    output_string);
			source = obs_source_create(INPUT_AUDIO_SOURCE,
						   output_name, settings, NULL);

			audio_device = obs_audio_device_new(
				OBS_AUDIO_DEVICE_OUTPUT, source);
			g_list_store_append(self->output_devices, audio_device);
			g_object_unref(audio_device);
		}
	}
	g_clear_pointer(&output_props, obs_properties_destroy);
}

static void reset_audio(ObsAudioController *self)
{
	struct obs_audio_info ai;

	/* TODO: knobs */
	ai.samples_per_sec = 48000;
	ai.speakers = SPEAKERS_STEREO;

	obs_reset_audio(&ai);
}

static void obs_audio_controller_finalize(GObject *object)
{
	ObsAudioController *self = (ObsAudioController *)object;

	g_assert(self->shutdown);

	g_clear_object(&self->input_devices);
	g_clear_object(&self->output_devices);
	g_clear_object(&self->all_devices);

	G_OBJECT_CLASS(obs_audio_controller_parent_class)->finalize(object);
}

static void obs_audio_controller_class_init(ObsAudioControllerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->finalize = obs_audio_controller_finalize;
}

static void obs_audio_controller_init(ObsAudioController *self)
{
	GListStore *aux;

	// TODO: React to audio device changes
	// TODO: Storage

	self->input_devices = g_list_store_new(OBS_TYPE_AUDIO_DEVICE);
	self->output_devices = g_list_store_new(OBS_TYPE_AUDIO_DEVICE);

	aux = g_list_store_new(G_TYPE_LIST_MODEL);
	g_list_store_append(aux, self->output_devices);
	g_list_store_append(aux, self->input_devices);

	self->all_devices =
		G_LIST_MODEL(gtk_flatten_list_model_new(G_LIST_MODEL(aux)));
}

ObsAudioController *obs_audio_controller_new(void)
{
	return g_object_new(OBS_TYPE_AUDIO_CONTROLLER, NULL);
}

void obs_audio_controller_initialize(ObsAudioController *self)
{
	g_return_if_fail(OBS_IS_AUDIO_CONTROLLER(self));

	populate_all_devices(self);
	reset_audio(self);
}

void obs_audio_controller_shutdown(ObsAudioController *self)
{
	g_return_if_fail(OBS_IS_AUDIO_CONTROLLER(self));

	g_list_store_remove_all(self->input_devices);
	g_list_store_remove_all(self->output_devices);

	self->shutdown = true;
}

GListModel *obs_audio_controller_get_input_devices(ObsAudioController *self)
{
	g_return_val_if_fail(OBS_IS_AUDIO_CONTROLLER(self), NULL);
	return G_LIST_MODEL(self->input_devices);
}

GListModel *obs_audio_controller_get_output_devices(ObsAudioController *self)
{
	g_return_val_if_fail(OBS_IS_AUDIO_CONTROLLER(self), NULL);
	return G_LIST_MODEL(self->output_devices);
}

GListModel *obs_audio_controller_get_devices(ObsAudioController *self)
{
	g_return_val_if_fail(OBS_IS_AUDIO_CONTROLLER(self), NULL);
	return self->all_devices;
}
