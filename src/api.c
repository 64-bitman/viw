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

#include "api.h"
#include "misc.h"
#include <assert.h>

/*
 * "seat.create(name)": Create a new seat and return seat object
 */
static int
l_seat_create(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);

    return 0;
}

/*
 * Register the global API table to the Lua state
 */
void
viw_register_api(struct viw_config *cfg)
{
    assert(cfg != NULL);

    static const luaL_Reg seat_funcs[] = {{"add", l_seat_create}, {NULL, NULL}};

    lua_State *L = cfg->L;

    lua_newtable(L); // -3

    // Create "viw.seat"
    lua_pushstring(L, "seat"); // -2
    lua_newtable(L);           // -1
    luaL_setfuncs(L, seat_funcs, 0);
    lua_rawset(L, -3);

    lua_setglobal(L, "viw");
}
