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
#include "log.h"
#include "mem.h"
#include "xdg.h"
#include <assert.h>
#include <lauxlib.h>
#include <lua.h>
#include <signal.h>

struct viw_config
{
    lua_State *L;
};

bool
viw_config_interface_init(struct viw_config_interface *iface)
{
    assert(iface != NULL);

    memset(iface, 0, sizeof(*iface));

    iface->config_path = xdg_get_base_dir(XDG_CONFIG_HOME, "viw");
    if (iface->config_path == NULL)
        return false;

    iface->event_loop = wl_event_loop_create();
    if (iface->event_loop == NULL)
    {
        viw_error("Error creating event loop for config");
        viw_free(iface->config_path);
        return false;
    }

    if (!viw_wakeup_init(&iface->cfg_wk, iface->event_loop))
    {
        wl_event_loop_destroy(iface->event_loop);
        viw_free(iface->config_path);
        return false;
    }

    iface->run = false;
    iface->init = true;
    pthread_mutex_init(&iface->run_mut, NULL);

    return true;
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

static void *
config_thread(void *udata)
{
    assert(udata != NULL);

    viw_debug("Config thread running");

    struct viw_config_interface *iface = udata;

    sigset_t block;

    // Must block all signals on this thread, so that signals go to the main
    // thread (libwayland uses sigprocmask()...)
    if (sigfillset(&block) == -1 ||
        pthread_sigmask(SIG_BLOCK, &block, NULL) == -1)
    {
        viw_error_errno("Error setting up signal mask for config thread");
        goto error;
    }

    struct viw_config cfg;

    cfg.L = lua_newstate(alloc_func, NULL, luaL_makeseed(NULL));
    if (cfg.L == NULL)
    {
        viw_error_errno("Error allocating lua state");
        goto error;
    }

    while (true)
    {
        bool run;

        pthread_mutex_lock(&iface->run_mut);
        run = iface->run;
        pthread_mutex_unlock(&iface->run_mut);

        if (!run)
            break;
        wl_event_loop_dispatch(iface->event_loop, -1);
    }

    lua_close(cfg.L);

    viw_debug("Config thread stopped");

    return NULL;
error:
    viw_debug("Error occured, config thread stopped");

    pthread_mutex_lock(&iface->run_mut);
    iface->run = false;
    pthread_mutex_unlock(&iface->run_mut);

    return NULL;
}

/*
 * Start the config thread
 */
bool
viw_config_interface_start(struct viw_config_interface *iface)
{
    assert(iface != NULL);

    iface->run = true;
    if (pthread_create(&iface->thread, NULL, config_thread, iface) != 0)
    {
        viw_error("Error creating config thread");
        iface->run = false;
        return false;
    }

    return true;
}

void
viw_config_interface_uninit(struct viw_config_interface *iface)
{
    assert(iface != NULL);

    if (!iface->init)
        return;

    if (iface->run)
    {
        pthread_mutex_lock(&iface->run_mut);
        iface->run = false;
        pthread_mutex_unlock(&iface->run_mut);

        viw_wakeup_wakeup(&iface->cfg_wk);
        pthread_join(iface->thread, NULL);
    }

    pthread_mutex_destroy(&iface->run_mut);
    viw_wakeup_uninit(&iface->cfg_wk);
    wl_event_loop_destroy(iface->event_loop);

    viw_free(iface->config_path);
}
