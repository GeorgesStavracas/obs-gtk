/* obs-config-manager.h
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
#include <util/config-file.h>

G_BEGIN_DECLS

#define OBS_TYPE_CONFIG_MANAGER (obs_config_manager_get_type())
G_DECLARE_FINAL_TYPE(ObsConfigManager, obs_config_manager, OBS, CONFIG_MANAGER,
		     GObject)

typedef enum {
	OBS_CONFIG_SCOPE_GLOBAL,
	OBS_CONFIG_SCOPE_PROFILE,
	OBS_CONFIG_SCOPE_SCENE_COLLECTION,
	OBS_CONFIG_SCOPE_PLUGIN,
} ObsConfigScope;

ObsConfigManager *obs_config_manager_new(void);
config_t *obs_config_manager_open(ObsConfigManager *self, ObsConfigScope scope,
				  const char *id,
				  enum config_open_type open_type);
void obs_config_manager_release(ObsConfigManager *self, ObsConfigScope scope,
				const char *id);

G_END_DECLS
