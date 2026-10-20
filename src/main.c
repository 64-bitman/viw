/*
 * viw
 * Copyright (C) 2026 Foxe Chen
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "log.h"
#include "viw.h"
#include "xdg.h"
#include <getopt.h>
#include <stdlib.h>

#ifdef __SANITIZE_ADDRESS__
const char *
__lsan_default_options(void)
{
    return "print_suppressions=0";
}
const char *
__lsan_default_suppressions(void)
{
    // Supress leaks from libgbm
    return "leak:pthread_once\n";
}
#endif

int
main(int argc, char **argv)
{
    static struct option options[] = {{NULL, 0, 0, 0}};

    int c, idx;

    viw_log_set(VIW_LOG_DEBUG);

    while ((c = getopt_long(argc, argv, "", options, &idx)) != -1)
    {
        switch (c)
        {
        default:
            return EXIT_FAILURE;
        }
    }

    if (!xdg_init())
        return EXIT_FAILURE;

    struct viw viw = {0}; // Initialize to zero so that we know what has been
                          // initialized when fatal error occurs.

    if (!viw_init(&viw))
        return EXIT_FAILURE;

    if (!viw_start(&viw))
    {
        viw_uninit(&viw);
        return EXIT_FAILURE;
    }

    viw_run(&viw);
    viw_uninit(&viw);

    xdg_uninit();

    return EXIT_SUCCESS;
}
