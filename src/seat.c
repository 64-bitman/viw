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

#include "seat.h"
#include "input.h" // IWYU pragma: keep
#include "log.h"
#include "viw.h"
#include <assert.h>

static void
seat_input_free(struct viw_seat_input *sinput)
{
    assert(sinput != NULL);

    wl_list_remove(&sinput->input_destroy.link);
    wl_list_remove(&sinput->new_event.link);
    wl_list_remove(&sinput->link);

    viw_free(sinput);
}

static void
event_input_destroy(struct wl_listener *listener, void *data UNUSED)
{
    struct viw_seat_input *sinput =
        wl_container_of(listener, sinput, input_destroy);

    seat_input_free(sinput);
}

static void
event_new_event(struct wl_listener *listener, void *data)
{
    struct viw_seat_input *sinput =
        wl_container_of(listener, sinput, input_destroy);
    struct viw_input_event *event = data;

    (void)event;
}

static void
seat_add_input(struct viw_seat *seat, struct viw_input *input)
{
    assert(seat != NULL);
    assert(input != NULL);

    viw_debug(
        "Adding input \"%s\" to seat \"%s\"",
        input->identifier,
        seat->global->name
    );

    struct viw_seat_input *sinput = viw_calloc(1, sizeof(*sinput));

    if (sinput == NULL)
        return;

    sinput->input = input;

    sinput->new_event.notify = event_new_event;
    wl_signal_add(&input->events.new_event, &sinput->new_event);

    sinput->input_destroy.notify = event_input_destroy;
    wl_signal_add(&input->events.input_destroy, &sinput->input_destroy);

    wl_list_insert(&seat->inputs, &sinput->link);
}

static void
event_input_added(struct wl_listener *listener, void *data)
{
    struct viw_seat_manager *smgr =
        wl_container_of(listener, smgr, input_added);
    struct viw_input *input = data;

    assert(smgr != NULL);

    // Add input device to all seats that have a match for its identifier
    struct viw_seat *seat;

    wl_list_for_each(seat, &smgr->seats, link)
    {
        if (seat->match_all)
        {
            seat_add_input(seat, input);
            continue;
        }

        for (size_t i = 0; i < array_len(&seat->input_patterns); i++)
        {
            pcre2_code *re = array_val(&seat->input_patterns, i);
            int         ret;

            ret = pcre2_match(
                re,
                (PCRE2_SPTR8)input->identifier,
                PCRE2_ZERO_TERMINATED,
                0,
                0,
                NULL,
                NULL
            );

            if (ret >= 0)
                seat_add_input(seat, input);
        }
    }
}

/*
 * Should be called after struct viw_input_manager is initialized
 */
void
viw_seat_manager_init(struct viw_seat_manager *smgr, struct viw *viw)
{
    assert(smgr != NULL);
    assert(viw != NULL);

    wl_list_init(&smgr->seats);

    smgr->viw = viw;

    smgr->input_added.notify = event_input_added;
    wl_signal_add(&viw->imgr.events.input_added, &smgr->input_added);

    viw_debug("Initialized seat manager");
    smgr->init = true;
}

/*
 * Should be called before input manager is uninitialized.
 */
void
viw_seat_manager_uninit(struct viw_seat_manager *smgr)
{
    assert(smgr != NULL);

    if (!smgr->init)
        return;

    wl_list_remove(&smgr->input_added.link);

    struct viw_seat *seat, *seat_tmp;

    wl_list_for_each_safe(seat, seat_tmp, &smgr->seats, link)
        viw_seat_remove(seat);

    viw_debug("Unitialized seat manager");
    smgr->init = false;
}

/*
 * If there are no seats currently, create a default seat called "seat0", that
 * matches all input devices. Returns false on failure.
 */
bool
viw_seat_manager_default(struct viw_seat_manager *smgr)
{
    assert(smgr != NULL);

    if (!wl_list_empty(&smgr->seats))
        return true;

    struct viw_seat *seat = viw_seat_add(smgr, "seat0");

    if (seat == NULL)
    {
        viw_error("Error adding default seat");
        return false;
    }

    viw_seat_add_input_pattern(seat, NULL);

    return true;
}

/*
 * Create a new seat with given name and return it. Returns NULL on failure.
 */
struct viw_seat *
viw_seat_add(struct viw_seat_manager *smgr, const char *name)
{
    assert(smgr != NULL);
    assert(name != NULL);

    struct viw_seat *seat = viw_calloc(1, sizeof(*seat));

    if (seat == NULL)
        return NULL;

    seat->global = wlr_seat_create(smgr->viw->display, name);

    wl_list_init(&seat->inputs);
    array_init(&seat->input_patterns);

    wl_list_insert(&smgr->seats, &seat->link);

    viw_debug("Added seat \"%s\"", name);

    return seat;
}

void
viw_seat_remove(struct viw_seat *seat)
{
    assert(seat != NULL);

    viw_debug("Removing seat \"%s\"", seat->global->name);

    for (size_t i = 0; i < array_len(&seat->input_patterns); i++)
        pcre2_code_free(array_val(&seat->input_patterns, i));
    array_clear(&seat->input_patterns);

    struct viw_seat_input *sinput, *sinput_tmp;

    wl_list_for_each_safe(sinput, sinput_tmp, &seat->inputs, link)
        seat_input_free(sinput);

    wlr_seat_destroy(seat->global);
    wl_list_remove(&seat->link);
    viw_free(seat);
}

/*
 * Add regex pattern "pat" to "seat". If "pat" is NULL, match all input devices.
 */
void
viw_seat_add_input_pattern(struct viw_seat *seat, const char *pat)
{
    assert(seat != NULL);

    if (pat == NULL)
    {
        seat->match_all = true;
        return;
    }

    array_grow(&seat->input_patterns, 1);
    if (!array_can(&seat->input_patterns, 1))
        return;

    int        errcode;
    PCRE2_SIZE erroff;

    pcre2_code *re = pcre2_compile(
        (PCRE2_SPTR8)pat, PCRE2_ZERO_TERMINATED, 0, &errcode, &erroff, NULL
    );

    if (re == NULL)
    {
        char buf[256];

        pcre2_get_error_message(errcode, (PCRE2_UCHAR8 *)buf, sizeof(buf));

        viw_warn(
            "Invalid input device pattern at character %zu: %s", erroff, buf
        );
        return;
    }

    array_append(&seat->input_patterns, re);
}
