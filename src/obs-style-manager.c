/* obs-style-manager.c
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

#include "obs-style-manager.h"

#include "obs-application.h"
#include "obs-config-manager.h"
#include "obs-utils.h"

#include <adwaita.h>

struct _ObsStyleManager
{
	GObject parent_instance;

	GtkStyleProvider *base_style_provider;
	GtkStyleProvider *dark_style_provider;
	GtkStyleProvider *hc_style_provider;
	GtkStyleProvider *hc_dark_style_provider;

	GPtrArray *styled_widgets;

	char *uri;
};

G_DEFINE_FINAL_TYPE (ObsStyleManager, obs_style_manager, G_TYPE_OBJECT)

static ObsStyleManager *default_instance = NULL;

static inline void
widget_style_provider_set_enabled (GtkWidget        *widget,
                                   GtkStyleProvider *provider,
                                   gboolean          enabled)
{
	GtkStyleContext *style_context = gtk_widget_get_style_context(widget);

	if (enabled) {
		gtk_style_context_add_provider(style_context, provider,
					       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION + 1);

	} else {
		gtk_style_context_remove_provider(style_context, provider);
	}
}


static void
update_widget_stylesheet(ObsStyleManager *self,
                         GtkWidget       *widget)

{
	GtkStyleProvider *base_style_provider;
	GtkStyleProvider *dark_style_provider;
	GtkStyleProvider *hc_style_provider;
	GtkStyleProvider *hc_dark_style_provider;
	AdwStyleManager *manager;
	gboolean is_dark, is_hc;

	manager = adw_style_manager_get_default();
	is_dark = adw_style_manager_get_dark(manager);
	is_hc = adw_style_manager_get_high_contrast(manager);

	base_style_provider = g_object_get_data(G_OBJECT(widget), "base");
	widget_style_provider_set_enabled(widget, base_style_provider, TRUE);

	dark_style_provider = g_object_get_data(G_OBJECT(widget), "base-dark");
	if (dark_style_provider)
		widget_style_provider_set_enabled(widget, dark_style_provider, is_dark);

	hc_style_provider = g_object_get_data(G_OBJECT(widget), "hc");
	if (hc_style_provider)
		widget_style_provider_set_enabled(widget, hc_style_provider, is_hc);

	hc_dark_style_provider = g_object_get_data(G_OBJECT(widget), "hc-dark");
	if (hc_dark_style_provider)
		widget_style_provider_set_enabled(widget, hc_dark_style_provider, is_hc && is_dark);
}

static void
update_widgets_stylesheets(ObsStyleManager *self)
{
	AdwStyleManager *manager = adw_style_manager_get_default();
	gboolean is_dark, is_hc;

	is_dark = adw_style_manager_get_dark(manager);
	is_hc = adw_style_manager_get_high_contrast(manager);

	for (unsigned int i = 0; i < self->styled_widgets->len; i++)
		update_widget_stylesheet(self, g_ptr_array_index(self->styled_widgets, i));
}

static inline void
style_provider_set_enabled (GtkStyleProvider *provider,
                            gboolean          enabled)
{
	GdkDisplay *display = gdk_display_get_default();

	if (enabled) {
		gtk_style_context_add_provider_for_display(display, provider,
							    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	} else {
		gtk_style_context_remove_provider_for_display(display, provider);
	}
}

static void
update_global_stylesheet(ObsStyleManager *self)
{
	ObsConfigManager *config_manager;
	AdwStyleManager *adw_style_manager;
	AdwColorScheme color_scheme;
	GApplication *application;
	config_t *global_config;
	gboolean is_dark, is_hc;

	adw_style_manager = adw_style_manager_get_default();
	color_scheme = adw_style_manager_get_color_scheme(adw_style_manager);
	is_dark = adw_style_manager_get_dark(adw_style_manager);
	is_hc = adw_style_manager_get_high_contrast(adw_style_manager);

	if (self->base_style_provider)
		style_provider_set_enabled(self->base_style_provider, TRUE);

	if (self->dark_style_provider)
		style_provider_set_enabled(self->dark_style_provider, is_dark);

	if (self->hc_style_provider)
		style_provider_set_enabled(self->hc_style_provider, is_hc);

	if (self->hc_dark_style_provider)
		style_provider_set_enabled(self->hc_dark_style_provider, is_hc && is_dark);

	/* Save theme and color scheme */
	application = g_application_get_default();
	config_manager = obs_application_get_config_manager(OBS_APPLICATION(application));

	global_config = obs_config_manager_open(config_manager,
						OBS_CONFIG_SCOPE_GLOBAL,
						"global",
						CONFIG_OPEN_ALWAYS);
	config_set_string(global_config, "General", "ColorScheme", obs_color_scheme_to_string(color_scheme));
	config_set_string(global_config, "General", "Theme", self->uri);
	config_save_safe(global_config, "tmp", NULL);
}

static void
init_provider_from_file(GtkStyleProvider **provider,
                        GFile             *file)
{
	if (!g_file_query_exists(file, NULL)) {
		g_clear_object (&file);
		return;
	}

	g_clear_object(provider);
	*provider = GTK_STYLE_PROVIDER(gtk_css_provider_new ());
	gtk_css_provider_load_from_file(GTK_CSS_PROVIDER (*provider), file);

	g_clear_object (&file);
}

static void
on_style_changed_cb(AdwStyleManager *style_manager,
                    GParamSpec      *pspec,
                    ObsStyleManager *self)
{
	update_global_stylesheet(self);
	update_widgets_stylesheets(self);
}

static void
widget_destroyed_cb(gpointer  data,
                    GObject  *where_the_object_was)
{
	ObsStyleManager *self = OBS_STYLE_MANAGER(data);
	g_ptr_array_remove(self->styled_widgets, where_the_object_was);
}

static void
obs_style_manager_dispose (GObject *object)
{
	ObsStyleManager *self = (ObsStyleManager *)object;

	g_clear_object(&self->base_style_provider);
	g_clear_object(&self->dark_style_provider);
	g_clear_object(&self->hc_style_provider);
	g_clear_object(&self->hc_dark_style_provider);

	g_clear_pointer(&self->styled_widgets, g_ptr_array_unref);
	g_clear_pointer(&self->uri, g_free);

	G_OBJECT_CLASS (obs_style_manager_parent_class)->dispose(object);
}

static void
obs_style_manager_class_init (ObsStyleManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = obs_style_manager_dispose;
}

static void
obs_style_manager_init (ObsStyleManager *self)
{
	self->styled_widgets = g_ptr_array_new();

	g_signal_connect_object (adw_style_manager_get_default(),
				 "notify",
				 G_CALLBACK(on_style_changed_cb),
				 self, 0);
}

ObsStyleManager *
obs_style_manager_get_default(void)
{
	if (!default_instance)
		default_instance = g_object_new (OBS_TYPE_STYLE_MANAGER, NULL);

	return default_instance;
}

void
obs_style_manager_load_style(ObsStyleManager  *self,
                             const char       *uri)
{
	GFile *base_file;

	g_return_if_fail(OBS_IS_STYLE_MANAGER(self));
	g_return_if_fail(uri != NULL);

	if (g_strcmp0(self->uri, uri) == 0)
		return;

	g_clear_pointer(&self->uri, g_free);
	self->uri = g_strdup(uri);

	if (self->base_style_provider)
		style_provider_set_enabled(self->base_style_provider, FALSE);
	if (self->dark_style_provider)
		style_provider_set_enabled(self->dark_style_provider, FALSE);
	if (self->hc_style_provider)
		style_provider_set_enabled(self->hc_style_provider, FALSE);
	if (self->hc_dark_style_provider)
		style_provider_set_enabled(self->hc_dark_style_provider, FALSE);

	base_file = g_file_new_for_uri(self->uri);

	init_provider_from_file(&self->base_style_provider,
				g_file_get_child(base_file, "style.css"));
	init_provider_from_file(&self->dark_style_provider,
				g_file_get_child(base_file, "style-dark.css"));
	init_provider_from_file(&self->hc_style_provider,
				g_file_get_child(base_file, "style-hc.css"));
	init_provider_from_file(&self->hc_dark_style_provider,
				g_file_get_child(base_file, "style-hc-dark.css"));

	update_global_stylesheet(self);

	g_object_unref(base_file);
}

const char *
obs_style_manager_get_style (ObsStyleManager *self)
{
	g_return_val_if_fail(OBS_IS_STYLE_MANAGER(self), NULL);

	return self->uri;
}

void
obs_style_manager_load_style_to_widget(ObsStyleManager *self,
                                       GtkWidget       *widget,
                                       const char      *uri)
{
	GtkStyleProvider *base_style_provider = NULL;
	GtkStyleProvider *dark_style_provider = NULL;
	GtkStyleProvider *hc_style_provider = NULL;
	GtkStyleProvider *hc_dark_style_provider = NULL;
	GFile *base_file;

	g_return_if_fail(OBS_IS_STYLE_MANAGER(self));
	g_return_if_fail(uri != NULL);

	base_file = g_file_new_for_uri(uri);

	init_provider_from_file(&base_style_provider,
				g_file_get_child(base_file, "style.css"));
	init_provider_from_file(&dark_style_provider,
				g_file_get_child(base_file, "style-dark.css"));
	init_provider_from_file(&hc_style_provider,
				g_file_get_child(base_file, "style-hc.css"));
	init_provider_from_file(&hc_dark_style_provider,
				g_file_get_child(base_file, "style-hc-dark.css"));

	g_object_set_data_full(G_OBJECT(widget), "base", base_style_provider, g_object_unref);
	g_object_set_data_full(G_OBJECT(widget), "base-dark", dark_style_provider, g_object_unref);
	g_object_set_data_full(G_OBJECT(widget), "hc", hc_style_provider, g_object_unref);
	g_object_set_data_full(G_OBJECT(widget), "hc-dark", hc_dark_style_provider, g_object_unref);

	g_ptr_array_add(self->styled_widgets, widget);

	update_widget_stylesheet(self, widget);

	g_object_weak_ref(G_OBJECT(widget), widget_destroyed_cb, self);

	g_object_unref(base_file);
}
