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
#include <assert.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

static char *XDG_CONFIG_HOME_PATH = NULL;
static char *XDG_DATA_HOME_PATH = NULL;

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
 * Should be called once on startup. Returns false on failure
 */
bool
xdg_init(void)
{
    struct
    {
        const char *env;
        const char *home_suffix;
        char      **var;
    } base_dirs[] = {
        [XDG_CONFIG_HOME] =
            {.env = getenv("XDG_CONFIG_HOME"),
             .home_suffix = ".config",
             .var = &XDG_CONFIG_HOME_PATH},
        [XDG_DATA_HOME] = {
            .env = getenv("XDG_DATA_HOME"),
            .home_suffix = ".local/share",
            .var = &XDG_DATA_HOME_PATH
        },
    };

    const char *home = NULL;
    bool        ok = true;

    for (int i = 0; i < N_ELEMENTS(base_dirs); i++)
    {
        assert(*base_dirs[i].var == NULL);
        if (base_dirs[i].env == NULL)
        {
            if (home == NULL)
            {
                home = get_home();

                if (home == NULL)
                {
                    viw_error("Failed getting directory of user home");
                    ok = false;
                    break;
                }
            }

            *base_dirs[i].var =
                viw_strdup_printf("%s/%s", home, base_dirs[i].home_suffix);
        }
        else
            *base_dirs[i].var = viw_strdup(base_dirs[i].env);

        if (*base_dirs[i].var == NULL)
        {
            viw_error("Error allocating XDG base directory paths");
            ok = false;
            break;
        }
    }

    if (!ok)
        for (int i = 0; i < N_ELEMENTS(base_dirs); i++)
            viw_free(*base_dirs[i].var);

    return ok;
}

void
xdg_uninit(void)
{
    viw_free(XDG_CONFIG_HOME_PATH);
    viw_free(XDG_DATA_HOME_PATH);
}

const char *
xdg_get_base_dir(enum xdg_base_directory type)
{
    assert(XDG_CONFIG_HOME_PATH != NULL);
    assert(XDG_DATA_HOME_PATH != NULL);

    switch (type)
    {
    case XDG_CONFIG_HOME:
        return XDG_CONFIG_HOME_PATH;
    case XDG_DATA_HOME:
        return XDG_DATA_HOME_PATH;
    default:
        viw_error("Unknown base directory %d", type);
        abort();
    }
}

char *
xdg_get_base_dir_child(enum xdg_base_directory type, const char *child)
{
    assert(child != NULL);

    return viw_strdup_printf("%s/%s", xdg_get_base_dir(type), child);
}
