/* swayclip
 * Copyright (C) 2026 Foxe Chen
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "xdg.h"
#include "log.h"
#include "mem.h"
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Get user home directory, returns NULL on failure.
 */
static const char *
get_home(void)
{
    const char *home = getenv("HOME");

    if (home == NULL)
    {
        struct passwd *pwd = getpwuid(getuid());

        if (pwd == NULL)
        {
            viw_error_errno("Error getting passwd entry");
            return NULL;
        }

        home = pwd->pw_dir;
    }

    return home;
}

/*
 * Return allocated string containing the specified base dir suffixed with
 * "child" (may be NULL). Returns NULL on failure.
 */
char *
xdg_get_base_dir(enum xdg_base_directory type, const char *child)
{
    const char *xdg;
    char       *dir;

    switch (type)
    {
    case XDG_CONFIG_HOME:
        xdg = getenv("XDG_CONFIG_HOME");
        break;
    case XDG_DATA_HOME:
        xdg = getenv("XDG_DATA_HOME");
        break;
    case XDG_RUNTIME_DIR:
        xdg = getenv("XDG_RUNTIME_DIR");
        break;
    default:
        viw_error("Unknown base directory %d", type);
        abort();
    }

    if (child == NULL)
        child = "";

    if (xdg == NULL)
    {
        if (type == XDG_RUNTIME_DIR)
            dir = viw_strdup_printf(
                "/run/user/%d%s%s", getuid(), *child == NUL ? "" : "/", child
            );
        else
        {
            const char *home = get_home();

            if (home == NULL)
                return NULL;

            if (type == XDG_CONFIG_HOME)
                dir = viw_strdup_printf("%s/.config/%s", home, child);
            else
                dir = viw_strdup_printf("%s/.local/share/%s", home, child);
        }
    }
    else
        dir = viw_strdup_printf("%s%s%s", xdg, *child == NUL ? "" : "/", child);

    if (dir == NULL)
        return NULL;

    return dir;
}
