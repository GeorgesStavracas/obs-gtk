/* obs-utils.c
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

#include "obs-utils.h"

static const struct {
	AdwColorScheme color_scheme;
	const char *string;
	AdwColorScheme alternative;
} color_schemes_map[] = {
	{ADW_COLOR_SCHEME_DEFAULT, "default", ADW_COLOR_SCHEME_DEFAULT},
	{ADW_COLOR_SCHEME_FORCE_LIGHT, "light", ADW_COLOR_SCHEME_FORCE_LIGHT},
	{ADW_COLOR_SCHEME_PREFER_LIGHT, "light", ADW_COLOR_SCHEME_FORCE_LIGHT},
	{ADW_COLOR_SCHEME_PREFER_DARK, "dark", ADW_COLOR_SCHEME_FORCE_DARK},
	{ADW_COLOR_SCHEME_FORCE_DARK, "dark", ADW_COLOR_SCHEME_FORCE_DARK},
};

AdwColorScheme obs_color_scheme_from_string(const char *string)
{
	for (size_t i = 0; i < G_N_ELEMENTS(color_schemes_map); i++) {
		if (g_strcmp0(string, color_schemes_map[i].string) == 0)
			return color_schemes_map[i].alternative;
	}

	return ADW_COLOR_SCHEME_FORCE_DARK;
}

const char *obs_color_scheme_to_string(AdwColorScheme color_scheme)
{
	return color_schemes_map[color_scheme].string;
}
