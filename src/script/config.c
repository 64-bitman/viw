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

#include "config.h"
#include "api.h"
#include "log.h"
#include "mem.h"
#include "xdg.h"
#include <assert.h>
#include <unistd.h>

static void
unload_config(struct viw_config *cfg)
{
    assert(cfg != NULL);

    if (cfg->L == NULL)
        return;

    lua_close(cfg->L);
    viw_debug("Unloaded config");
}

static void *
alloc_func(void *ud UNUSED, void *ptr, size_t osize UNUSED, size_t nsize)
{
    if (nsize == 0)
    {
        viw_free(ptr);
        return NULL;
    }
    else
        return viw_realloc(ptr, nsize);
}

static bool
load_config(struct viw_config *cfg)
{
    assert(cfg != NULL);

    lua_State *L = lua_newstate(alloc_func, NULL);

    if (L == NULL)
    {
        viw_error("Error allocating Lua state");
        return false;
    }

    luaL_openlibs(L);

    lua_pushlightuserdata(L, cfg);
    lua_setfield(L, LUA_REGISTRYINDEX, VIW_CONFIG_LUA);

    // Create global API table
    viw_register_api(L, cfg);

    // Run init.lua
    if (luaL_loadfilex(L, cfg->init_lua, "t") != LUA_OK ||
        lua_pcall(L, 0, 0, 0) != LUA_OK)
    {
        viw_error("Error loading config: %s", lua_tostring(L, -1));
        lua_close(L);
        return false;
    }

    unload_config(cfg);
    cfg->L = L;

    viw_debug("Loaded config \"%s\"", cfg->config_dir);

    return true;
}

bool
viw_config_init(struct viw_config *cfg, struct viw *viw)
{
    assert(cfg != NULL);

    viw_debug("Initializing config");

    cfg->config_dir = xdg_get_base_dir_child(XDG_CONFIG_HOME, "viw");
    cfg->init_lua = viw_strdup_printf("%s/init.lua", cfg->config_dir);
    if (cfg->config_dir == NULL || cfg->init_lua == NULL)
        goto fail;

    if (!load_config(cfg))
        goto fail;

    cfg->viw = viw;

    return true;
fail:
    viw_debug("Failed initializing config");
    viw_free(cfg->init_lua);
    viw_free(cfg->config_dir);
    return false;
}

void
viw_config_uninit(struct viw_config *cfg)
{
    assert(cfg != NULL);

    if (cfg->viw == NULL)
        return;

    viw_free(cfg->init_lua);
    viw_free(cfg->config_dir);
    unload_config(cfg);
    cfg->viw = NULL;
}

/*
 * Get the struct viw_config from the Lua state
 */
struct viw_config *
get_viw_config(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, VIW_CONFIG_LUA);
    struct viw_config *cfg = lua_touserdata(L, -1);
    lua_pop(L, 1);
    return cfg;
}

/*
 * Get the struct viw from the Lua state
 */
struct viw *
get_viw(lua_State *L)
{
    return ((struct viw_config *)get_viw_config(L))->viw;
}
