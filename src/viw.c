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

#include "viw.h"
#include "log.h"
#include <assert.h>
#include <signal.h>
#include <string.h>

bool // Suppress leak from wlroots
viw_init(struct viw *viw)
{
    assert(viw != NULL);

    viw_debug("Initializing viw state");
    memset(viw, 0, sizeof(*viw));

    viw->display = wl_display_create();
    if (viw->display == NULL)
    {
        viw_error("Error creating Wayland display object");
        goto fail;
    }

    viw->event_loop = wl_display_get_event_loop(viw->display);
    viw->backend = wlr_backend_autocreate(viw->event_loop, NULL);
    if (viw->backend == NULL)
    {
        viw_error("Error creating backend object");
        goto fail;
    }

    viw->renderer = wlr_renderer_autocreate(viw->backend);
    if (viw->renderer == NULL)
    {
        viw_error("Error creating renderer object");
        goto fail;
    }

    wlr_renderer_init_wl_display(viw->renderer, viw->display);

    viw->allocator = wlr_allocator_autocreate(viw->backend, viw->renderer);
    if (viw->allocator == NULL)
    {
        viw_error("Error creating allocator object");
        goto fail;
    }

    if (!viw_wakeup_init(&viw->main_wk, viw->event_loop))
        goto fail;

    return true;
fail:
    viw_uninit(viw);
    return false;
}

void
viw_uninit(struct viw *viw)
{
    assert(viw != NULL);

    // Must be called before freeing/releasing anything!
    viw_wakeup_uninit(&viw->main_wk);

    viw_debug("Uninitializing viw state");

    if (viw->allocator != NULL)
        wlr_allocator_destroy(viw->allocator);

    if (viw->renderer != NULL)
        wlr_renderer_destroy(viw->renderer);

    if (viw->backend != NULL)
        wlr_backend_destroy(viw->backend);

    if (viw->display != NULL)
        wl_display_destroy(viw->display);
}

/*
 * Create Wayland socket and start backend. Return false on failure
 */
bool
viw_start(struct viw *viw)
{
    assert(viw != NULL);

    viw_debug("Starting server");

    viw->socket = wl_display_add_socket_auto(viw->display);
    if (viw->socket == NULL)
    {
        viw_error_errno("Error adding Wayland socket");
        return false;
    }

    if (!wlr_backend_start(viw->backend))
    {
        viw_error("Error starting backend");
        return false;
    }

    return true;
}

static int
signal_handler(int signo UNUSED, void *udata)
{
    struct viw *viw = udata;

    viw_info("Exiting...");
    wl_display_terminate(viw->display);
    return 0;
}

void
viw_run(struct viw *viw)
{
    assert(viw != NULL);

    struct wl_event_source *sig_sources[2] = {
        wl_event_loop_add_signal(viw->event_loop, SIGINT, signal_handler, viw),
        wl_event_loop_add_signal(viw->event_loop, SIGTERM, signal_handler, viw),
    };

    viw_info("Running server");

    wl_display_run(viw->display);

    for (int i = 0; i < N_ELEMENTS(sig_sources); i++)
        if (sig_sources[i] != NULL)
            wl_event_source_remove(sig_sources[i]);
}
