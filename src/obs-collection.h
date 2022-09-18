/* obs-collection.h
 *
 * Copyright 2022 Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#pragma once

#include <glib-object.h>
#include <util/config-file.h>

G_BEGIN_DECLS

#define OBS_TYPE_COLLECTION (obs_collection_get_type())
G_DECLARE_FINAL_TYPE(ObsCollection, obs_collection, OBS, COLLECTION, GObject)

ObsCollection *obs_collection_new(const char *id, config_t *config);

const char *obs_collection_get_id(ObsCollection *self);

void obs_collection_set_name(ObsCollection *self, const char *name);
const char *obs_collection_get_name(ObsCollection *self);

G_END_DECLS
