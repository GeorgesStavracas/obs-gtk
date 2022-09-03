/* obs-audio-source-controls.h
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

#include <adwaita.h>

#include "obs-types.h"

G_BEGIN_DECLS

#define OBS_TYPE_AUDIO_DEVICE_CONTROLS (obs_audio_device_controls_get_type())
G_DECLARE_FINAL_TYPE(ObsAudioDeviceControls, obs_audio_device_controls, OBS,
		     AUDIO_DEVICE_CONTROLS, AdwBin)

GtkWidget *obs_audio_device_controls_new(ObsAudioDevice *audio_device);
ObsAudioDevice *
obs_audio_device_controls_get_audio_device(ObsAudioDeviceControls *self);
void obs_audio_device_controls_set_audio_device(ObsAudioDeviceControls *self,
						ObsAudioDevice *audio_device);

G_END_DECLS
