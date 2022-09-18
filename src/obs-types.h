/* obs-types.h
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

#include <glib.h>
#include <obs.h>

G_BEGIN_DECLS

typedef struct _ObsAudioController ObsAudioController;
typedef struct _ObsAudioDevice ObsAudioDevice;
typedef struct _ObsCollection ObsCollection;
typedef struct _ObsCollectionTemplate ObsCollectionTemplate;
typedef struct _ObsConfigManager ObsConfigManager;
typedef struct _ObsStyleManager ObsStyleManager;
typedef struct _ObsTemplatesManager ObsTemplatesManager;

G_END_DECLS
