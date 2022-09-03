/* obs-audio-controller.h
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

#include <gio/gio.h>

G_BEGIN_DECLS

#define OBS_TYPE_AUDIO_CONTROLLER (obs_audio_controller_get_type())
G_DECLARE_FINAL_TYPE(ObsAudioController, obs_audio_controller, OBS,
		     AUDIO_CONTROLLER, GObject)

ObsAudioController *obs_audio_controller_new(void);
void obs_audio_controller_initialize(ObsAudioController *self);
void obs_audio_controller_shutdown(ObsAudioController *self);
GListModel *obs_audio_controller_get_input_devices(ObsAudioController *self);
GListModel *obs_audio_controller_get_output_devices(ObsAudioController *self);
GListModel *obs_audio_controller_get_devices(ObsAudioController *self);

G_END_DECLS
