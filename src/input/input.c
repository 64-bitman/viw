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
#include "keyboard.h"
#include "log.h"
#include "mem.h"
#include "viw.h"
#include <assert.h>
#include <ctype.h>
#include <libinput.h>
#include <wlr/backend/libinput.h>
#include <wlr/config.h>
#include <xkbcommon/xkbcommon.h>

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

    wl_signal_emit(&input->events.destroy, NULL);
    assert(wl_list_empty(&input->events.destroy.listener_list));

    wl_list_remove(&input->destroy.link);
    wl_list_remove(&input->link);
    viw_free(input->identifier);
    viw_free(input);
}

static void
event_viw_input_destroy(struct wl_listener *listener, void *data UNUSED)
{
    struct viw_input *input = wl_container_of(listener, input, destroy);

    assert(input != NULL);

    viw_debug("Input \"%s\" removed", input->identifier);
    viw_input_destroy(input);
}

static void
event_new_input(struct wl_listener *listener, void *data)
{
    struct viw_input_manager *imgr = wl_container_of(listener, imgr, new_input);
    struct wlr_input_device  *device = data;

    assert(imgr != NULL);

    struct viw_input *input = viw_calloc(1, sizeof(*input));

    if (input == NULL)
        return;

    switch (device->type)
    {
    case WLR_INPUT_DEVICE_KEYBOARD:
        input->handle.keyboard = wlr_keyboard_from_input_device(device);
        viw_input_configure_keyboard(input);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        input->handle.pointer = wlr_pointer_from_input_device(device);
        break;
    default:
        viw_warn("Unhandled input device \"%s\"", device->name);
        break;
    }

    input->identifier = input_device_get_identifier(device);
    if (input->identifier == NULL)
    {
        viw_free(input);
        return;
    }

    input->device = device;
    input->viw = imgr->viw;
    wl_signal_init(&input->events.destroy);

    input->destroy.notify = event_viw_input_destroy;
    wl_signal_add(&input->device->events.destroy, &input->destroy);

    viw_debug("New input device \"%s\"", input->identifier);
    wl_signal_emit(&imgr->events.new_input, input);

    wl_list_insert(&imgr->inputs, &input->link);
}

void
viw_input_manager_init(struct viw_input_manager *imgr, struct viw *viw)
{
    assert(imgr != NULL);
    assert(viw != NULL);

    wl_list_init(&imgr->inputs);
    wl_list_init(&imgr->seats);

    imgr->new_input.notify = event_new_input;
    wl_signal_add(&viw->backend->events.new_input, &imgr->new_input);

    wl_signal_init(&imgr->events.new_input);

    viw_debug("Initialized input manager");
    imgr->viw = viw;
}

void
viw_input_manager_uninit(struct viw_input_manager *imgr)
{
    assert(imgr != NULL);

    if (imgr->viw == NULL)
        return;

    assert(wl_list_empty(&imgr->events.new_input.listener_list));

    wl_list_remove(&imgr->new_input.link);

    viw_debug("Uninitialized input manager");
    imgr->viw = NULL;
}
