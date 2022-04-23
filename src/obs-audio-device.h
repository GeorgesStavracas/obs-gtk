/* obs-audio-device.h
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

#pragma once

#include <glib-object.h>
#include <obs.h>

G_BEGIN_DECLS

typedef enum {
	OBS_AUDIO_DEVICE_INPUT,
	OBS_AUDIO_DEVICE_OUTPUT,
} ObsAudioDeviceType;

#define OBS_TYPE_AUDIO_DEVICE (obs_audio_device_get_type())
G_DECLARE_FINAL_TYPE (ObsAudioDevice, obs_audio_device, OBS, AUDIO_DEVICE, GObject)

ObsAudioDevice     *obs_audio_device_new             (ObsAudioDeviceType  device_type,
                                                      obs_source_t       *source);
ObsAudioDeviceType  obs_audio_device_get_device_type (ObsAudioDevice     *self);
obs_source_t       *obs_audio_device_get_source      (ObsAudioDevice     *self);
const char         *obs_audio_device_get_name        (ObsAudioDevice     *self);
float               obs_audio_device_get_volume      (ObsAudioDevice     *self);
void                obs_audio_device_set_volume      (ObsAudioDevice     *self,
                                                      float               volume);
gboolean            obs_audio_device_get_muted       (ObsAudioDevice     *self);
void                obs_audio_device_set_muted       (ObsAudioDevice     *self,
                                                      gboolean            muted);

G_END_DECLS
