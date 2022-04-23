/* obs-theme-card.c
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

#include "obs-theme-card.h"

#include "obs-style-manager.h"

struct _ObsThemeCard
{
	GtkFlowBoxChild parent_instance;

	GtkWidget *card;
	GtkLabel *name_label;
};

G_DEFINE_FINAL_TYPE (ObsThemeCard, obs_theme_card, GTK_TYPE_FLOW_BOX_CHILD)

static void
obs_theme_card_class_init (ObsThemeCardClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/obsproject/Studio/GTK4/ui/preferences/obs-theme-card.ui");

	gtk_widget_class_bind_template_child(widget_class, ObsThemeCard, card);
	gtk_widget_class_bind_template_child(widget_class, ObsThemeCard, name_label);

	gtk_widget_class_set_css_name(widget_class, "themepreviewcard");
}

static void
obs_theme_card_init (ObsThemeCard *self)
{
	gtk_widget_init_template (GTK_WIDGET (self));
}

GtkWidget *
obs_theme_card_new (const char *name,
                    const char *theme_path)
{
	ObsStyleManager *style_manager;
	ObsThemeCard *theme_card;

	theme_card = g_object_new (OBS_TYPE_THEME_CARD, NULL);
	gtk_label_set_label(theme_card->name_label, name);

	style_manager = obs_style_manager_get_default();
	obs_style_manager_load_style_to_widget(style_manager,
					       theme_card->card,
					       theme_path);

	return GTK_WIDGET(theme_card);
}
