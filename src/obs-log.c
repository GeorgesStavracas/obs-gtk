/* obs-log.c
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

#include "obs-log.h"
#include "obs-debug.h"

#include <obs.h>

#include <unistd.h>
#include <glib.h>

G_LOCK_DEFINE_STATIC (channel_lock);

GIOChannel *standard_channel = NULL;

static const char* ignored_domains[] =
{
  "GdkPixbuf",
  NULL
};

static const char *
log_level_str (GLogLevelFlags log_level)
{
	switch (((gulong)log_level & G_LOG_LEVEL_MASK)) {
	case G_LOG_LEVEL_ERROR:
		return "   \033[1;31mERROR\033[0m";
	case G_LOG_LEVEL_CRITICAL:
		return "\033[1;35mCRITICAL\033[0m";
	case G_LOG_LEVEL_WARNING:
		return " \033[1;33mWARNING\033[0m";
	case G_LOG_LEVEL_MESSAGE:
		return " \033[1;34mMESSAGE\033[0m";
	case G_LOG_LEVEL_INFO:
		return "    \033[1;32mINFO\033[0m";
	case G_LOG_LEVEL_DEBUG:
		return "   \033[1;32mDEBUG\033[0m";
	case OBS_LOG_LEVEL_TRACE:
		return "   \033[1;36mTRACE\033[0m";
	default:
		return " UNKNOWN";
	}
}

static void
obs_log_handler(const char     *domain,
		GLogLevelFlags  log_level,
		const char     *message,
		gpointer        user_data)
{
	g_autofree char *buffer = NULL;
	const char *level;

	/* Skip ignored log domains */
	if (domain && g_strv_contains (ignored_domains, domain))
		return;

	level = log_level_str (log_level);
	buffer = g_strdup_printf ("%s: %s: %s\n", domain, level, message);

	/* Safely write to the channel */
	G_LOCK (channel_lock);

	g_io_channel_write_chars (standard_channel, buffer, -1, NULL, NULL);
	g_io_channel_flush (standard_channel, NULL);

	G_UNLOCK (channel_lock);
}

static void log_obs_message_func(int         log_level,
                                 const char *message,
                                 va_list     args,
                                 void       *data)
{
	GLogLevelFlags g_log_level = G_LOG_LEVEL_MESSAGE;

	switch (log_level) {
	case LOG_DEBUG:
		g_log_level = G_LOG_LEVEL_DEBUG;
		break;
	case LOG_INFO:
		g_log_level = G_LOG_LEVEL_INFO;
		break;
	case LOG_WARNING:
		g_log_level = G_LOG_LEVEL_WARNING;
		break;
	case LOG_ERROR:
		g_log_level = G_LOG_LEVEL_ERROR;
		break;
	}

	g_logv("libobs", g_log_level, message, args);
}

void obs_log_init(void)
{
	static gsize initialized = FALSE;

	if (g_once_init_enter (&initialized)) {
		standard_channel = g_io_channel_unix_new (STDOUT_FILENO);
		g_log_set_default_handler (obs_log_handler, NULL);
		g_once_init_leave (&initialized, TRUE);
		base_set_log_handler(log_obs_message_func, NULL);
	}
}
