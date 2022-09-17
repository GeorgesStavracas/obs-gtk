/* obs-application.c
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

#include "obs-application.h"

#include "obs-audio-controller.h"
#include "obs-config-manager.h"
#include "obs-log.h"
#include "obs-style-manager.h"
#include "obs-templates-manager.h"
#include "obs-utils.h"
#include "obs-window.h"

#include <libpanel.h>
#include <locale.h>

#include <obs.h>
#include <util/platform.h>

#if defined(__linux__)
#include <obs-nix-platform.h>
#endif

#ifdef GDK_WINDOWING_X11
#include <gdk/x11/gdkx.h>
#endif
#ifdef GDK_WINDOWING_WAYLAND
#include <gdk/wayland/gdkwayland.h>
#endif

#define PLUGIN_CONFIG_PATH "obs-studio/plugin_config"

#define DEFAULT_COLOR_SCHEME \
	(obs_color_scheme_to_string(ADW_COLOR_SCHEME_FORCE_DARK))
#define DEFAULT_LANGUAGE "en-US"
#define DEFAULT_THEME "resource:///com/obsproject/Studio/GTK4/styles/default"

struct _ObsApplication {
	AdwApplication parent_instance;

	ObsWindow *window;

	profiler_name_store_t *profiler;
	ObsConfigManager *config_manager;
	ObsAudioController *audio_controller;
	ObsTemplatesManager *templates_manager;

	char *locale;
};

G_DEFINE_FINAL_TYPE(ObsApplication, obs_application, ADW_TYPE_APPLICATION)

typedef struct {
	obs_task_t task;
	void *param;
	GMainLoop *mainloop;
} ui_task_t;

static gboolean run_ui_task_cb(gpointer user_data)
{
	ui_task_t *ui_task = user_data;

	ui_task->task(ui_task->param);

	if (ui_task->mainloop) {
		g_main_loop_quit(ui_task->mainloop);
		g_clear_pointer(&ui_task->mainloop, g_main_loop_unref);
	}

	return G_SOURCE_REMOVE;
}

static void ui_task_handler(obs_task_t task, void *param, bool wait)
{
	ui_task_t *ui_task;

	ui_task = g_new(ui_task_t, 1);
	ui_task->task = task;
	ui_task->param = param;
	ui_task->mainloop = wait ? g_main_loop_new(NULL, FALSE) : NULL;

	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, run_ui_task_cb, ui_task,
			g_free);

	if (wait)
		g_main_loop_run(ui_task->mainloop);
}

static void load_global_defaults(ObsApplication *self)
{
	config_t *config;

	config = obs_config_manager_open(self->config_manager,
					 OBS_CONFIG_SCOPE_GLOBAL, "global",
					 CONFIG_OPEN_ALWAYS);

	config_set_default_string(config, "General", "ColorScheme",
				  DEFAULT_COLOR_SCHEME);
	config_set_default_string(config, "General", "Theme", DEFAULT_THEME);

	config_set_default_bool(config, "General", "FirstRun", true);

#if 0
	config_set_default_uint(config, "General", "MaxLogs", 10);
	config_set_default_int(config, "General", "InfoIncrement", -1);
	config_set_default_string(config, "General", "ProcessPriority", "Normal");
	config_set_default_bool(config, "General", "EnableAutoUpdates", true);
	config_set_default_bool(config, "General", "ConfirmOnExit", true);

#if _WIN32
	config_set_default_string(config, "Video", "Renderer", "Direct3D 11");
#else
	config_set_default_string(config, "Video", "Renderer", "OpenGL");
#endif

	config_set_default_bool(config, "BasicWindow", "PreviewEnabled", true);
	config_set_default_bool(config, "BasicWindow", "PreviewProgramMode", false);
	config_set_default_bool(config, "BasicWindow", "SceneDuplicationMode", true);
	config_set_default_bool(config, "BasicWindow", "SwapScenesMode", true);
	config_set_default_bool(config, "BasicWindow", "SnappingEnabled", true);
	config_set_default_bool(config, "BasicWindow", "ScreenSnapping", true);
	config_set_default_bool(config, "BasicWindow", "SourceSnapping", true);
	config_set_default_bool(config, "BasicWindow", "CenterSnapping", false);
	config_set_default_double(config, "BasicWindow", "SnapDistance", 10.0);
	config_set_default_bool(config, "BasicWindow", "RecordWhenStreaming", false);
	config_set_default_bool(config, "BasicWindow", "KeepRecordingWhenStreamStops", false);
	config_set_default_bool(config, "BasicWindow", "SysTrayEnabled", true);
	config_set_default_bool(config, "BasicWindow", "SysTrayWhenStarted", false);
	config_set_default_bool(config, "BasicWindow", "SaveProjectors", false);
	config_set_default_bool(config, "BasicWindow", "ShowTransitions", true);
	config_set_default_bool(config, "BasicWindow", "ShowListboxToolbars", true);
	config_set_default_bool(config, "BasicWindow", "ShowStatusBar", true);
	config_set_default_bool(config, "BasicWindow", "ShowSourceIcons", true);
	config_set_default_bool(config, "BasicWindow", "ShowContextToolbars", true);
	config_set_default_bool(config, "BasicWindow", "StudioModeLabels", true);

	config_set_default_string(config, "General", "HotkeyFocusType", "NeverDisableHotkeys");

	config_set_default_bool(config, "BasicWindow", "VerticalVolControl", false);
	config_set_default_bool(config, "BasicWindow", "MultiviewMouseSwitch", true);
	config_set_default_bool(config, "BasicWindow", "MultiviewDrawNames", true);
	config_set_default_bool(config, "BasicWindow", "MultiviewDrawAreas", true);

#ifdef _WIN32
	uint32_t winver = GetWindowsVersion();

	config_set_default_bool(config, "Audio", "DisableAudioDucking", true);
	config_set_default_bool(config, "General", "BrowserHWAccel", winver > 0x601);
#endif

#ifdef __APPLE__
	config_set_default_bool(config, "General", "BrowserHWAccel", true);
	config_set_default_bool(config, "Video", "DisableOSXVSync", true);
	config_set_default_bool(config, "Video", "ResetOSXVSyncOnExit", true);
#endif

	config_set_default_bool(config, "BasicWindow", "MediaControlsCountdownTimer", true);
#endif
}

static char *
massage_locale_to_something_that_makes_libobs_happy(const char *locale)
{
	GStrv split;
	char *happy_locale;
	char *aux;

	split = g_strsplit(locale, ".", 2);
	happy_locale = g_strdup(split[0]);

	aux = happy_locale;
	while ((aux = g_strrstr(aux, "_")) != NULL)
		*aux = '-';

	g_strfreev(split);

	return happy_locale;
}

static void update_locale(ObsApplication *self)
{
	const char *locale;
	config_t *config;

	config = obs_config_manager_open(self->config_manager,
					 OBS_CONFIG_SCOPE_GLOBAL, "global",
					 CONFIG_OPEN_ALWAYS);
	locale = config_get_string(config, "General", "Locale");

	if (locale) {
		setlocale(LC_ALL, locale);
		self->locale = g_strdup(locale);
	} else {
		locale = setlocale(LC_MESSAGES, "");
		if (!locale)
			locale = setlocale(LC_ALL, "");
		if (!locale)
			locale = DEFAULT_LANGUAGE;

		self->locale =
			massage_locale_to_something_that_makes_libobs_happy(
				locale);
	}
	blog(LOG_INFO, "Locale: %s", self->locale);
}

static void load_theme(ObsApplication *self)
{
	AdwStyleManager *adw_style_manager;
	ObsStyleManager *obs_style_manager;
	const char *color_scheme;
	const char *theme;
	config_t *config;

	config = obs_config_manager_open(self->config_manager,
					 OBS_CONFIG_SCOPE_GLOBAL, "global",
					 CONFIG_OPEN_ALWAYS);
	theme = config_get_string(config, "General", "Theme");
	color_scheme = config_get_string(config, "General", "ColorScheme");

	assert(theme != NULL);

	adw_style_manager = adw_style_manager_get_default();
	adw_style_manager_set_color_scheme(
		adw_style_manager, obs_color_scheme_from_string(color_scheme));

	obs_style_manager = obs_style_manager_get_default();
	obs_style_manager_load_style(obs_style_manager, theme);
}

static void obs_application_startup(GApplication *application)
{
	ObsApplication *self = OBS_APPLICATION(application);
	char path[512];

	profiler_start();
	profile_register_root("obs_application_startup", 0);

	G_APPLICATION_CLASS(obs_application_parent_class)->startup(application);
	panel_init();

	obs_templates_manager_load_templates(self->templates_manager);

	obs_log_init();
	obs_set_nix_platform(OBS_NIX_PLATFORM_DRM);

	if (os_get_config_path(path, sizeof(path), PLUGIN_CONFIG_PATH) < 0) {
		g_application_quit(application);
		return;
	}

	obs_startup(self->locale, path, self->profiler);
	obs_set_ui_task_handler(ui_task_handler);

	obs_load_all_modules();
	obs_log_loaded_modules();
	obs_post_load_modules();

	load_theme(self);

	obs_audio_controller_initialize(self->audio_controller);

	gdk_gl_context_clear_current();
}

static void obs_application_activate(GApplication *application)
{
	ObsApplication *self = OBS_APPLICATION(application);

	G_APPLICATION_CLASS(obs_application_parent_class)->activate(application);

	if (self->window == NULL)
		self->window = obs_window_new(application);
	gtk_window_present(GTK_WINDOW(self->window));
}

static void obs_application_shutdown(GApplication *application)
{
	ObsApplication *self = OBS_APPLICATION(application);
	profiler_snapshot_t *snapshot;

	obs_audio_controller_shutdown(self->audio_controller);

	obs_shutdown();

	G_APPLICATION_CLASS(obs_application_parent_class)->shutdown(application);

	snapshot = profile_snapshot_create();
	profiler_print(snapshot);
	profiler_print_time_between_calls(snapshot);
	profiler_free();
}

static void obs_application_finalize(GObject *object)
{
	ObsApplication *self = OBS_APPLICATION(object);

	g_clear_object(&self->audio_controller);
	g_clear_pointer(&self->profiler, profiler_name_store_free);
	g_clear_pointer(&self->locale, g_free);
	g_clear_object(&self->config_manager);

	G_OBJECT_CLASS(obs_application_parent_class)->finalize(object);
}

static void obs_application_class_init(ObsApplicationClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GApplicationClass *application_class = G_APPLICATION_CLASS(klass);

	object_class->finalize = obs_application_finalize;

	application_class->startup = obs_application_startup;
	application_class->activate = obs_application_activate;
	application_class->shutdown = obs_application_shutdown;
}

static void obs_application_init(ObsApplication *self)
{
	self->profiler = profiler_name_store_create();
	self->config_manager = obs_config_manager_new();
	self->audio_controller = obs_audio_controller_new();
	self->templates_manager = obs_templates_manager_new();

	obs_log_init();

	load_global_defaults(self);
	update_locale(self);
}

GApplication *obs_application_new(void)
{
	return g_object_new(OBS_TYPE_APPLICATION, "application-id",
			    "com.obsproject.Studio.GTK4", NULL);
}

ObsConfigManager *obs_application_get_config_manager(ObsApplication *self)
{
	g_return_val_if_fail(OBS_IS_APPLICATION(self), NULL);

	return self->config_manager;
}

ObsAudioController *obs_application_get_audio_controller(ObsApplication *self)
{
	g_return_val_if_fail(OBS_IS_APPLICATION(self), NULL);

	return self->audio_controller;
}

ObsTemplatesManager *obs_application_get_templates_manager(ObsApplication *self)
{
	g_return_val_if_fail(OBS_IS_APPLICATION(self), NULL);

	return self->templates_manager;
}
