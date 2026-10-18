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

#include "misc.h"
#include "log.h"

/*
 * Get time for given clock in nanoseconds. Returns 0 on failure
 */
int64_t
get_time_ns(clockid_t id)
{
    struct timespec tp;

    if (clock_gettime(id, &tp) == -1)
    {
        viw_warn("clock_gettime(%d) failed", id);
        return 0;
    }

    return (int64_t)tp.tv_sec * 1000000000LL + tp.tv_nsec;
}
