/* obs-audio-peak-tracker.c
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

#include "obs-audio-peak-tracker.h"

#define IS_NAN(x) (isnan(x) != 0)
#define IS_INFINITE(x) (isfinite(x) == 0)

static inline double us_to_s(int64_t ns)
{
	return ns / 1000000.0;
}

static inline int64_t ms_to_us(int64_t ms)
{
	return ms * 1000;
}

static void reset_peak_tracker_unlocked(ObsAudioPeakTracker *self)
{
	self->last_update_time_us = 0;

	for (size_t i = 0; i < MAX_AUDIO_CHANNELS; i++) {
		self->magnitudes[i] = -INFINITY;
		self->peaks[i] = -INFINITY;
		self->input_peaks[i] = -INFINITY;

		self->display.magnitudes[i] = -INFINITY;
		self->display.peaks[i] = -INFINITY;
		self->display.peak_holds[i] = -INFINITY;
		self->display.peak_hold_update_times_us[i] = 0;
		self->display.input_peaks[i] = -INFINITY;
		self->display.input_peak_holds[i] = -INFINITY;
		self->display.input_peak_hold_update_times_us[i] = 0;
	}
}

void obs_audio_peak_tracker_init(ObsAudioPeakTracker *self)
{
	g_mutex_init(&self->mutex);
}

void obs_audio_peak_tracker_clear(ObsAudioPeakTracker *self)
{
	g_mutex_clear(&self->mutex);
}

void obs_audio_peak_tracker_start(ObsAudioPeakTracker *self, double decay_rate,
				  double minimum_level_db,
				  double magnitude_integration_time,
				  int64_t peak_hold_duration_ms,
				  int64_t input_peak_hold_duration_ms)
{
	g_mutex_lock(&self->mutex);

	self->decay_rate = decay_rate;
	self->minimum_level_db = minimum_level_db;
	self->magnitude_integration_time = magnitude_integration_time;
	self->peak_hold_duration_ms = peak_hold_duration_ms;
	self->input_peak_hold_duration_ms = input_peak_hold_duration_ms;

	reset_peak_tracker_unlocked(self);

	g_mutex_unlock(&self->mutex);
}

void obs_audio_peak_tracker_tick(ObsAudioPeakTracker *self, int64_t now_us,
				 int64_t ellapsed_time_us)
{
	double ellapsed_time_s = us_to_s(ellapsed_time_us);

	g_mutex_lock(&self->mutex);

	for (int i = 0; i < MAX_AUDIO_CHANNELS; i++) {
		if (IS_NAN(self->display.peaks[i]) ||
		    self->peaks[i] >= self->display.peaks[i]) {
			// Attack of peak is immediate.
			self->display.peaks[i] = self->peaks[i];
		} else {
			// Decay of peak is 40 dB / 1.7 seconds for Fast Profile
			// 20 dB / 1.7 seconds for Medium Profile (Type I PPM)
			// 24 dB / 2.8 seconds for Slow Profile (Type II PPM)
			double decay = self->decay_rate * ellapsed_time_s;
			self->display.peaks[i] =
				CLAMP(self->display.peaks[i] - decay,
				      self->peaks[i], 0.0);
		}

		if (IS_INFINITE(self->display.peak_holds[i]) ||
		    self->peaks[i] >= self->display.peak_holds[i]) {
			// The peak and hold falls back to peak after 20 seconds.
			int64_t ellapsed_peak_time_us =
				now_us -
				self->display.peak_hold_update_times_us[i];

			if (ellapsed_peak_time_us >
			    ms_to_us(self->peak_hold_duration_ms)) {
				self->display.peak_holds[i] = self->peaks[i];
				self->display.peak_hold_update_times_us[i] =
					now_us;
			}
		} else {
			// Attack of peak-hold is immediate, but keep track when it was last updated
			self->display.peak_holds[i] = self->peaks[i];
			self->display.peak_hold_update_times_us[i] = now_us;
		}

		if (IS_INFINITE(self->display.input_peaks[i]) ||
		    self->input_peaks[i] >= self->display.input_peaks[i]) {
			// Attack of peak-hold is immediate, but keep track when it was last updated
			self->display.input_peaks[i] = self->input_peaks[i];
			self->display.input_peak_hold_update_times_us[i] =
				now_us;
		} else {
			// The peak and hold falls back to peak after 1 second.
			int64_t ellapsed_input_peak_time_us =
				now_us -
				self->display.input_peak_hold_update_times_us[i];

			if (ellapsed_input_peak_time_us >
			    ms_to_us(self->input_peak_hold_duration_ms)) {
				self->display.input_peaks[i] =
					self->input_peaks[i];
				self->display
					.input_peak_hold_update_times_us[i] =
					now_us;
			}
		}

		if (IS_INFINITE(self->display.magnitudes[i])) {
			self->display.magnitudes[i] = self->magnitudes[i];
		} else {
			// A VU meter will integrate to the new value to 99% in 300 ms.
			// The calculation here is very simplified and is more accurate
			// with higher frame-rate.
			float attack = (self->magnitudes[i] -
					self->display.magnitudes[i]) *
				       (ellapsed_time_s /
					self->magnitude_integration_time) *
				       0.99;

			self->display.magnitudes[i] =
				CLAMP(self->display.magnitudes[i] + attack,
				      (float)self->minimum_level_db, 0);
		}
	}
	g_mutex_unlock(&self->mutex);
}

void obs_audio_peak_tracker_reset(ObsAudioPeakTracker *self)
{
	g_mutex_lock(&self->mutex);
	reset_peak_tracker_unlocked(self);
	g_mutex_unlock(&self->mutex);
}

void obs_audio_peak_tracker_set_levels(ObsAudioPeakTracker *self,
				       int64_t update_time_us,
				       const float *magnitudes,
				       const float *peaks,
				       const float *input_peaks,
				       size_t n_channels)
{
	g_mutex_lock(&self->mutex);

	self->last_update_time_us = update_time_us;

	memcpy(self->magnitudes, magnitudes, n_channels);
	memcpy(self->peaks, peaks, n_channels);
	memcpy(self->input_peaks, input_peaks, n_channels);

	g_mutex_unlock(&self->mutex);
}

void obs_audio_peak_tracker_get_levels(ObsAudioPeakTracker *self,
				       uint32_t channel, float *out_magnitude,
				       float *out_peak, float *out_input_peak)
{
	if (channel > MAX_AUDIO_CHANNELS)
		return;

	if (out_magnitude) {
		if (!IS_INFINITE(self->display.magnitudes[channel]))
			*out_magnitude = self->display.magnitudes[channel];
		else
			*out_magnitude = 0.0;
	}

	if (out_peak) {
		if (!IS_INFINITE(self->display.peaks[channel]))
			*out_peak = self->display.peaks[channel];
		else
			*out_peak = 0.0;
	}

	if (out_input_peak) {
		if (!IS_INFINITE(self->display.input_peaks[channel]))
			*out_input_peak = self->display.input_peaks[channel];
		else
			*out_input_peak = 0.0;
	}
}

gboolean obs_audio_peak_tracker_is_idle(ObsAudioPeakTracker *self,
					int64_t now_us)
{
	return now_us - self->last_update_time_us > ms_to_us(500);
}
