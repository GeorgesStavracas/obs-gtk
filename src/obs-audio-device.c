/* obs-audio-device.c
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

#include "obs-audio-device.h"

struct _ObsAudioDevice {
	GObject parent_instance;

	GMutex mutex;

	ObsAudioDeviceType device_type;
	obs_source_t *source;
	float volume;
	gboolean muted;

	guint idle_update_id;
};

G_DEFINE_FINAL_TYPE(ObsAudioDevice, obs_audio_device, G_TYPE_OBJECT)

enum {
	PROP_0,
	PROP_DEVICE_TYPE,
	PROP_NAME,
	PROP_SOURCE,
	PROP_VOLUME,
	PROP_MUTED,
	N_PROPS,
};

static GParamSpec *properties[N_PROPS];

static gboolean update_in_idle_cb(gpointer data)
{
	ObsAudioDevice *self = OBS_AUDIO_DEVICE(data);

	g_mutex_lock(&self->mutex);

	if (!G_APPROX_VALUE(self->volume, obs_source_get_volume(self->source),
			    FLT_EPSILON)) {
		self->volume = obs_source_get_volume(self->source);
		g_object_notify_by_pspec(G_OBJECT(self),
					 properties[PROP_VOLUME]);
	}

	if (self->muted != obs_source_muted(self->source)) {
		self->muted = obs_source_muted(self->source);
		g_object_notify_by_pspec(G_OBJECT(self),
					 properties[PROP_MUTED]);
	}

	self->idle_update_id = 0;

	g_mutex_unlock(&self->mutex);

	return G_SOURCE_REMOVE;
}

static void on_source_changed_cb(void *data, calldata_t *calldata)
{
	ObsAudioDevice *self = OBS_AUDIO_DEVICE(data);

	g_mutex_lock(&self->mutex);

	if (self->idle_update_id == 0)
		self->idle_update_id = g_idle_add(update_in_idle_cb, self);

	g_mutex_unlock(&self->mutex);
}

static void obs_audio_device_dispose(GObject *object)
{
	ObsAudioDevice *self = OBS_AUDIO_DEVICE(object);

	g_clear_handle_id(&self->idle_update_id, g_source_remove);
	g_clear_pointer(&self->source, obs_source_release);

	G_OBJECT_CLASS(obs_audio_device_parent_class)->dispose(object);
}

static void obs_audio_device_get_property(GObject *object, guint prop_id,
					  GValue *value, GParamSpec *pspec)
{
	ObsAudioDevice *self = OBS_AUDIO_DEVICE(object);

	switch (prop_id) {
	case PROP_DEVICE_TYPE:
		g_value_set_int(value, self->device_type);
		break;
	case PROP_MUTED:
		g_value_set_boolean(value, self->muted);
		break;
	case PROP_NAME:
		g_value_set_string(value, obs_source_get_name(self->source));
		break;
	case PROP_SOURCE:
		g_value_set_pointer(value, self->source);
		break;
	case PROP_VOLUME:
		g_value_set_float(value, self->volume);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void obs_audio_device_set_property(GObject *object, guint prop_id,
					  const GValue *value,
					  GParamSpec *pspec)
{
	ObsAudioDevice *self = OBS_AUDIO_DEVICE(object);

	switch (prop_id) {
	case PROP_DEVICE_TYPE:
		self->device_type = g_value_get_int(value);
		break;
	case PROP_MUTED:
		obs_audio_device_set_muted(self, g_value_get_boolean(value));
		break;
	case PROP_SOURCE:
		self->source = obs_source_get_ref(g_value_get_pointer(value));
		signal_handler_connect(
			obs_source_get_signal_handler(self->source), "mute",
			on_source_changed_cb, self);
		signal_handler_connect(
			obs_source_get_signal_handler(self->source), "volume",
			on_source_changed_cb, self);
		obs_audio_device_set_muted(self,
					   obs_source_muted(self->source));
		obs_audio_device_set_volume(
			self, obs_source_get_volume(self->source));
		break;
	case PROP_VOLUME:
		obs_audio_device_set_volume(self, g_value_get_float(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void obs_audio_device_class_init(ObsAudioDeviceClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->dispose = obs_audio_device_dispose;
	object_class->get_property = obs_audio_device_get_property;
	object_class->set_property = obs_audio_device_set_property;

	properties[PROP_DEVICE_TYPE] =
		g_param_spec_int("device-type", NULL, NULL, 0, G_MAXINT, 0,
				 G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
					 G_PARAM_STATIC_STRINGS);

	properties[PROP_MUTED] = g_param_spec_boolean(
		"muted", NULL, NULL, FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_NAME] =
		g_param_spec_string("name", NULL, NULL, NULL,
				    G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	properties[PROP_SOURCE] = g_param_spec_pointer(
		"source", NULL, NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
			G_PARAM_STATIC_STRINGS);

	properties[PROP_VOLUME] =
		g_param_spec_float("volume", NULL, NULL, 0.0, 1.0, 0.0,
				   G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, N_PROPS, properties);
}

static void obs_audio_device_init(ObsAudioDevice *self)
{
	g_mutex_init(&self->mutex);
}

ObsAudioDevice *obs_audio_device_new(ObsAudioDeviceType device_type,
				     obs_source_t *source)
{
	return g_object_new(OBS_TYPE_AUDIO_DEVICE, "device-type", device_type,
			    "source", source, NULL);
}

ObsAudioDeviceType obs_audio_device_get_device_type(ObsAudioDevice *self)
{
	g_return_val_if_fail(OBS_IS_AUDIO_DEVICE(self), 0);
	return self->device_type;
}

obs_source_t *obs_audio_device_get_source(ObsAudioDevice *self)
{
	g_return_val_if_fail(OBS_IS_AUDIO_DEVICE(self), NULL);
	return self->source;
}

const char *obs_audio_device_get_name(ObsAudioDevice *self)
{
	g_return_val_if_fail(OBS_IS_AUDIO_DEVICE(self), NULL);
	return obs_source_get_name(self->source);
}

float obs_audio_device_get_volume(ObsAudioDevice *self)
{
	g_return_val_if_fail(OBS_IS_AUDIO_DEVICE(self), 0.0);
	return self->volume;
}

void obs_audio_device_set_volume(ObsAudioDevice *self, float volume)
{
	g_return_if_fail(OBS_IS_AUDIO_DEVICE(self));

	if (G_APPROX_VALUE(self->volume, volume, FLT_EPSILON))
		return;

	self->volume = volume;
	obs_source_set_volume(self->source, volume);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_VOLUME]);
}

gboolean obs_audio_device_get_muted(ObsAudioDevice *self)
{
	g_return_val_if_fail(OBS_IS_AUDIO_DEVICE(self), FALSE);
	return self->muted;
}

void obs_audio_device_set_muted(ObsAudioDevice *self, gboolean muted)
{
	g_return_if_fail(OBS_IS_AUDIO_DEVICE(self));

	if (self->muted == muted)
		return;

	self->muted = muted;
	obs_source_set_muted(self->source, muted);
	g_object_notify_by_pspec(G_OBJECT(self), properties[PROP_MUTED]);
}
