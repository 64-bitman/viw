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
#include <assert.h>
#include <stdio.h>

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

#ifdef DEBUG
/*
 * Useful for debugging
 */
void
util_lua_dump_stack(lua_State *L)
{
    assert(L != NULL);

    int i;
    int top = lua_gettop(L);

    printf("<Lua stack top>\n");

    for (i = top; i >= 1; i--)
    {
        int t = lua_type(L, i);

        printf("(%d|%d): ", i, i - top - 1);
        switch (t)
        {
        case LUA_TSTRING:
            printf("(string, '%s')", lua_tostring(L, i));
            break;
        case LUA_TBOOLEAN:
            printf("(boolean, '%s')", lua_toboolean(L, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            printf("(string, '%lf')", lua_tonumber(L, i));
            break;
        default:
            printf("(%s)", lua_typename(L, t));
            break;
        }
        if (i > 1)
            printf("\n");
    }
    printf("\n<Lua stack bottom>\n");
}
#endif
