/* obs-collection-template-icon.h
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

#define OBS_TYPE_COLLECTION_TEMPLATE_ICON (obs_collection_template_icon_get_type())
G_DECLARE_FINAL_TYPE (ObsCollectionTemplateIcon, obs_collection_template_icon,
		      OBS, COLLECTION_TEMPLATE_ICON,
		      GtkFlowBoxChild)

GtkWidget * obs_collection_template_icon_new (ObsCollectionTemplate *collection_template);

ObsCollectionTemplate *
obs_collection_template_icon_get_collection_template (ObsCollectionTemplateIcon *self);

G_END_DECLS
