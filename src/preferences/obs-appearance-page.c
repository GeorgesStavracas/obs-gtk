/* obs-appearance-page.c
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

#include "obs-appearance-page.h"

#include "obs-style-manager.h"
#include "obs-theme-card.h"

#include <glib/gi18n.h>

struct _ObsAppearancePage
{
	AdwBin parent_instance;

	GtkCheckButton *dark;
	GtkCheckButton *follow;
	GtkCheckButton *light;
	GtkFlowBox *themes_flowbox;
};


static void on_builtin_themes_flowbox_selected_changed_cb(GtkFlowBox        *flowbox,
                                                          ObsAppearancePage *self);

G_DEFINE_FINAL_TYPE (ObsAppearancePage, obs_appearance_page, ADW_TYPE_BIN)

static const struct {
	const char *name;
	const char *path;
} builtin_styles[] = {
	{
		.name = N_("Default"),
		.path = "resource:///com/obsproject/Studio/GTK4/styles/default",
	},
	{
		.name = N_("Sakura"),
		.path = "resource:///com/obsproject/Studio/GTK4/styles/sakura",
	},
};

static void
init_default_styles(ObsAppearancePage *self)
{
	ObsStyleManager *style_manager = obs_style_manager_get_default();
	const char *style = obs_style_manager_get_style(style_manager);

	for (size_t i = 0; i < G_N_ELEMENTS(builtin_styles); i++) {
		GtkWidget *theme_card;

		theme_card = obs_theme_card_new(_(builtin_styles[i].name),
						builtin_styles[i].path);

		gtk_flow_box_append(self->themes_flowbox, theme_card);

		if (g_strcmp0(style, builtin_styles[i].path) == 0) {
			g_signal_handlers_block_by_func(self->themes_flowbox,
							on_builtin_themes_flowbox_selected_changed_cb,
							self);

			gtk_flow_box_select_child(self->themes_flowbox, GTK_FLOW_BOX_CHILD(theme_card));

			g_signal_handlers_unblock_by_func(self->themes_flowbox,
							  on_builtin_themes_flowbox_selected_changed_cb,
							  self);
		}
	}
}

static void
on_builtin_themes_flowbox_selected_changed_cb(GtkFlowBox        *flowbox,
                                              ObsAppearancePage *self)
{
	GList *selected = gtk_flow_box_get_selected_children(flowbox);

	if (selected) {
		ObsStyleManager *style_manager = obs_style_manager_get_default();
		int position = gtk_flow_box_child_get_index(selected->data);

		obs_style_manager_load_style(style_manager, builtin_styles[position].path);
	}

	g_list_free(selected);
}

static void
on_check_active_changed_cb (GtkWidget         *check,
                            GParamSpec        *pspec,
                            ObsAppearancePage *self)
{
        AdwStyleManager *style_manager = adw_style_manager_get_default();

	if (gtk_check_button_get_active(self->dark))
		adw_style_manager_set_color_scheme(style_manager, ADW_COLOR_SCHEME_FORCE_DARK);
	else if (gtk_check_button_get_active(self->light))
		adw_style_manager_set_color_scheme(style_manager, ADW_COLOR_SCHEME_FORCE_LIGHT);
	else if (gtk_check_button_get_active(self->follow))
		adw_style_manager_set_color_scheme(style_manager, ADW_COLOR_SCHEME_DEFAULT);
}

static void
obs_appearance_page_class_init (ObsAppearancePageClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(widget_class, "/com/obsproject/Studio/GTK4/ui/preferences/obs-appearance-page.ui");

	gtk_widget_class_bind_template_child(widget_class, ObsAppearancePage, dark);
	gtk_widget_class_bind_template_child(widget_class, ObsAppearancePage, follow);
	gtk_widget_class_bind_template_child(widget_class, ObsAppearancePage, light);
	gtk_widget_class_bind_template_child(widget_class, ObsAppearancePage, themes_flowbox);

	gtk_widget_class_bind_template_callback(widget_class, on_builtin_themes_flowbox_selected_changed_cb);
	gtk_widget_class_bind_template_callback(widget_class, on_check_active_changed_cb);
}

static void
obs_appearance_page_init (ObsAppearancePage *self)
{
        AdwStyleManager *style_manager = adw_style_manager_get_default();

	gtk_widget_init_template (GTK_WIDGET (self));

	switch(adw_style_manager_get_color_scheme(style_manager)) {
	case ADW_COLOR_SCHEME_DEFAULT:
		gtk_check_button_set_active(self->follow, TRUE);
		break;
	case ADW_COLOR_SCHEME_PREFER_DARK:
	case ADW_COLOR_SCHEME_FORCE_DARK:
		gtk_check_button_set_active(self->dark, TRUE);
		break;
	case ADW_COLOR_SCHEME_PREFER_LIGHT:
	case ADW_COLOR_SCHEME_FORCE_LIGHT:
		gtk_check_button_set_active(self->light, TRUE);
		break;
	}

	init_default_styles(self);
}
