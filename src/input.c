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

#include "input.h"
#include "log.h"
#include "mem.h"
#include "viw.h"
#include <assert.h>
#include <ctype.h>
#include <libinput.h>
#include <wlr/backend/libinput.h>
#include <wlr/config.h>

/*
 * Return an allocated string identifying this input device. Returns NULL on
 * failure.
 */
static char *
input_device_get_identifier(struct wlr_input_device *device)
{
    assert(device != NULL);

    int vendor = 0, product = 0;

#ifdef WLR_HAS_LIBINPUT_BACKEND
    if (wlr_input_device_is_libinput(device))
    {
        struct libinput_device *libinput_dev =
            wlr_libinput_get_device_handle(device);
        vendor = libinput_device_get_id_vendor(libinput_dev);
        product = libinput_device_get_id_product(libinput_dev);
    }
#endif

    char *str = viw_strdup_printf("%d:%d:%s", vendor, product, device->name);

    if (str == NULL)
        return NULL;

    // Sanitize string
    for (char *c = str; *c != NUL; c++)
        if (isspace(*c) || !isprint(*c))
            *c = '_';

    return str;
}

static void
viw_input_destroy(struct viw_input *input)
{
    assert(input != NULL);

    switch (input->device->type)
    {
    default:
        break;
    }

    wl_list_remove(&input->destroy.link);
    wl_list_remove(&input->link);
    viw_free(input->identifier);
    viw_free(input);
}

static void
input_destroy(struct wl_listener *listener, void *data UNUSED)
{
    struct viw_input *input = wl_container_of(listener, input, destroy);

    assert(input != NULL);

    viw_debug("Input \"%s\" removed", input->identifier);
    viw_input_destroy(input);
}

static void
new_input(struct wl_listener *listener, void *data)
{
    struct viw_input_manager *imgr = wl_container_of(listener, imgr, new_input);
    struct wlr_input_device  *device = data;

    assert(imgr != NULL);

    struct viw_input *input = viw_calloc(1, sizeof(*input));

    if (input == NULL)
        return;

    input->identifier = input_device_get_identifier(device);
    if (input->identifier == NULL)
    {
        viw_free(input);
        return;
    }

    viw_debug("New input device \"%s\"", input->identifier);

    input->device = device;

    input->destroy.notify = input_destroy;
    wl_signal_add(&input->device->events.destroy, &input->destroy);

    wl_list_insert(&imgr->inputs, &input->link);
}

void
viw_input_manager_init(struct viw *viw, struct viw_input_manager *imgr)
{
    assert(imgr != NULL);

    wl_list_init(&imgr->inputs);
    wl_list_init(&imgr->seats);

    imgr->new_input.notify = new_input;
    wl_signal_add(&viw->backend->events.new_input, &imgr->new_input);

    viw_debug("Initialized input manager");
    imgr->init = true;
}

void
viw_input_manager_uninit(struct viw_input_manager *imgr)
{
    assert(imgr != NULL);

    wl_list_remove(&imgr->new_input.link);

    viw_debug("Uninitialized input manager");
    imgr->init = false;
}
