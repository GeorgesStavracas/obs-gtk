/* obs-profiles-dialog.c
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

#include "obs-profiles-dialog.h"

struct _ObsProfilesDialog {
	AdwWindow parent_instance;
};

G_DEFINE_FINAL_TYPE(ObsProfilesDialog, obs_profiles_dialog, ADW_TYPE_WINDOW)

static void obs_profiles_dialog_class_init(ObsProfilesDialogClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/com/obsproject/Studio/GTK4/ui/profiles/obs-profiles-dialog.ui");
}

static void obs_profiles_dialog_init(ObsProfilesDialog *self)
{
	gtk_widget_init_template(GTK_WIDGET(self));
}

GtkWindow *obs_profiles_dialog_new(void)
{
	return g_object_new(OBS_TYPE_PROFILES_DIALOG, NULL);
}
