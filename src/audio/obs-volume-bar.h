/* obs-volume-bar.h
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

#include <gtk/gtk.h>

#include "obs-types.h"

G_BEGIN_DECLS

#define OBS_TYPE_VOLUME_BAR (obs_volume_bar_get_type())
G_DECLARE_FINAL_TYPE(ObsVolumeBar, obs_volume_bar, OBS, VOLUME_BAR, GtkWidget)

GtkWidget *obs_volume_bar_new(void);
void obs_volume_bar_get_range(ObsVolumeBar *self, double *out_minimum,
			      double *out_maximum);
void obs_volume_bar_set_range(ObsVolumeBar *self, double minimum,
			      double maximum);
double obs_volume_bar_get_value(ObsVolumeBar *self);
void obs_volume_bar_set_value(ObsVolumeBar *self, double value);
double obs_volume_bar_get_magnitude(ObsVolumeBar *self);
void obs_volume_bar_set_magnitude(ObsVolumeBar *self, double magnitude);

G_END_DECLS
