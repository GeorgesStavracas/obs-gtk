/* obs-display-renderer.h
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

G_BEGIN_DECLS

#define OBS_TYPE_DISPLAY_RENDERER (obs_display_renderer_get_type())
G_DECLARE_FINAL_TYPE (ObsDisplayRenderer, obs_display_renderer, OBS, DISPLAY_RENDERER, GtkMediaStream)

typedef void (*ObsDisplayDrawFunc)(void *param, uint32_t width, uint32_t height);

ObsDisplayRenderer *obs_display_renderer_new                  (void);
void                obs_display_renderer_add_draw_callback    (ObsDisplayRenderer *self,
                                                               ObsDisplayDrawFunc  draw_func,
                                                               gpointer            user_data);
void                obs_display_renderer_remove_draw_callback (ObsDisplayRenderer *self,
                                                               ObsDisplayDrawFunc  draw_func,
                                                               gpointer            user_data);

G_END_DECLS
