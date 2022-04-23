/* obs-collection-template.h
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

#define OBS_TYPE_COLLECTION_TEMPLATE (obs_collection_template_get_type())
G_DECLARE_FINAL_TYPE (ObsCollectionTemplate, obs_collection_template, OBS, COLLECTION_TEMPLATE, GObject)

ObsCollectionTemplate *obs_collection_template_new           (const char             *id,
                                                              const char             *name,
                                                              GError                **error);
const char            *obs_collection_template_get_id        (ObsCollectionTemplate  *self);
const char            *obs_collection_template_get_name      (ObsCollectionTemplate  *self);
GdkPaintable          *obs_collection_template_get_icon      (ObsCollectionTemplate  *self);
void                   obs_collection_template_create_async  (ObsCollectionTemplate  *self,
                                                              const char             *collection_name,
                                                              GCancellable           *cancellable,
                                                              GAsyncReadyCallback     callback,
                                                              gpointer                user_data);
gpointer               obs_collection_template_create_finish (ObsCollectionTemplate  *self,
                                                              GAsyncResult           *result,
                                                              GError                **error);


G_END_DECLS
