/* obs-audio-peak-tracker.h
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

typedef struct {
	GMutex mutex;
	double decay_rate; // DB per second
	double minimum_level_db;
	double magnitude_integration_time; // % per milissecond
	int64_t peak_hold_duration_ms;
	int64_t input_peak_hold_duration_ms;

	int64_t last_update_time_us;

	float magnitudes[MAX_AUDIO_CHANNELS];
	float peaks[MAX_AUDIO_CHANNELS];
	float input_peaks[MAX_AUDIO_CHANNELS];

	struct {
		float magnitudes[MAX_AUDIO_CHANNELS];

		float peaks[MAX_AUDIO_CHANNELS];
		float peak_holds[MAX_AUDIO_CHANNELS];
		int64_t peak_hold_update_times_us[MAX_AUDIO_CHANNELS];

		float input_peaks[MAX_AUDIO_CHANNELS];
		float input_peak_holds[MAX_AUDIO_CHANNELS];
		int64_t input_peak_hold_update_times_us[MAX_AUDIO_CHANNELS];
	} display;

} ObsAudioPeakTracker;

void     obs_audio_peak_tracker_init       (ObsAudioPeakTracker *self);
void     obs_audio_peak_tracker_clear      (ObsAudioPeakTracker *self);
void     obs_audio_peak_tracker_start      (ObsAudioPeakTracker *self,
                                            double               decay_rate,
                                            double               minimum_level_db,
                                            double               magnitude_integration_time,
                                            int64_t              peak_hold_duration_us,
                                            int64_t              input_peak_hold_duration_us);
void     obs_audio_peak_tracker_tick       (ObsAudioPeakTracker *self,
                                            int64_t              now_us,
                                            int64_t              ellapsed_time_us);
void     obs_audio_peak_tracker_reset      (ObsAudioPeakTracker *self);
void     obs_audio_peak_tracker_set_levels (ObsAudioPeakTracker *self,
                                            int64_t              update_time_us,
                                            const float         *magnitudes,
                                            const float         *peaks,
                                            const float         *input_peaks,
                                            size_t               n_channels);
void     obs_audio_peak_tracker_get_levels (ObsAudioPeakTracker *self,
                                            uint32_t             channel,
                                            float               *out_magnitude,
                                            float               *out_peak,
                                            float               *out_input_peak);
gboolean obs_audio_peak_tracker_is_idle    (ObsAudioPeakTracker *self,
                                            int64_t              now_us);

G_END_DECLS
