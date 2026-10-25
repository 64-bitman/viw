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

#include "output.h"
#include "log.h"
#include "viw.h"
#include <assert.h>

static void
viw_output_destroy(struct viw_output *out)
{
    assert(out != NULL);

    viw_debug("Output \"%s\" removed", out->output->name);

    wl_signal_emit(&out->events.destroy, NULL);
    assert(wl_list_empty(&out->events.destroy.listener_list));

    wl_list_remove(&out->destroy.link);
    wl_list_remove(&out->link);
    viw_free(out);
}

static void
event_wlr_output_destroy(struct wl_listener *listener, void *data UNUSED)
{
    struct viw_output *out = wl_container_of(listener, out, destroy);

    viw_output_destroy(out);
}

static void
event_wlr_backend_new_output(struct wl_listener *listener, void *data)
{
    struct viw_output_manager *omgr =
        wl_container_of(listener, omgr, new_output);
    struct wlr_output *output = data;

    viw_debug(
        "New output \"%s\" of size %dx%d",
        output->name,
        output->width,
        output->height
    );

    struct viw_output *out = viw_calloc(1, sizeof(*out));

    if (out == NULL)
    {
        viw_error("Error allocating output object");
        return;
    }

    out->mgr = omgr;
    out->output = output;

    wl_signal_init(&out->events.destroy);

    out->destroy.notify = event_wlr_output_destroy;
    wl_signal_add(&output->events.destroy, &out->destroy);

    wl_list_insert(&omgr->outputs, &out->link);
}

bool
viw_output_manager_init(struct viw_output_manager *omgr, struct viw *viw)
{
    assert(omgr != NULL);
    assert(viw != NULL);

    omgr->output_layout = wlr_output_layout_create(viw->display);
    if (omgr->output_layout == NULL)
    {
        viw_error("Error creating output layout object");
        return false;
    }

    omgr->viw = viw;
    wl_list_init(&omgr->outputs);

    omgr->new_output.notify = event_wlr_backend_new_output;
    wl_signal_add(&viw->backend->events.new_output, &omgr->new_output);

    viw_debug("Initialized output manager");

    return true;
}

void
viw_output_manager_uninit(struct viw_output_manager *omgr)
{
    assert(omgr != NULL);

    if (omgr->output_layout == NULL)
        return;

    struct viw_output *out, *out_tmp;

    wl_list_for_each_safe(out, out_tmp, &omgr->outputs, link)
        viw_output_destroy(out);

    wl_list_remove(&omgr->new_output.link);
    wlr_output_layout_destroy(omgr->output_layout);

    viw_debug("Uninitialized output manager");
    omgr->viw = NULL;
}
