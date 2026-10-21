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

    switch (input->device->type)
    {
    case WLR_INPUT_DEVICE_KEYBOARD:
    {
        struct viw_keyboard *kb = (struct viw_keyboard *)input;

        wl_list_remove(&kb->key.link);
        wl_list_remove(&kb->modifiers.link);
        break;
    case WLR_INPUT_DEVICE_POINTER:
    {
        break;
    }
    }
    default:
        viw_error("Unknown input device %d", input->device->type);
        abort();
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
event_key(struct wl_listener *listener, void *data)
{
    struct viw_keyboard           *kb = wl_container_of(listener, kb, key);
    struct wlr_keyboard_key_event *event = data;

    assert(kb != NULL);

    (void)event; // TODO
}

static void
event_modifiers(struct wl_listener *listener, void *data UNUSED)
{
    struct viw_keyboard *kb = wl_container_of(listener, kb, key);

    assert(kb != NULL);
}

static bool
configure_keyboard(struct viw_keyboard *kb)
{
    assert(kb != NULL);
    assert(kb->base.device->type == WLR_INPUT_DEVICE_KEYBOARD);

    struct xkb_context *ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    if (ctx == NULL)
    {
        viw_error_errno("Error creating xkb context");
        return false;
    }

    // TODO allow configuring of xkb keymap + use xkb file
    struct xkb_keymap *keymap = xkb_keymap_new_from_names2(
        ctx, NULL, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS
    );

    xkb_context_unref(ctx);

    if (keymap == NULL)
    {
        viw_error_errno("Error creating xkb keymap");
        return false;
    }

    kb->keyboard = wlr_keyboard_from_input_device(kb->base.device);

    wlr_keyboard_set_keymap(kb->keyboard, keymap);
    xkb_keymap_unref(keymap); // Above call takes reference

    wlr_keyboard_set_repeat_info(kb->keyboard, 30, 200); // TODO

    kb->key.notify = event_key;
    wl_signal_add(&kb->keyboard->events.key, &kb->key);

    kb->key.notify = event_modifiers;
    wl_signal_add(&kb->keyboard->events.modifiers, &kb->modifiers);

    return true;
}

static bool
configure_pointer(struct viw_pointer *pr)
{
    assert(pr != NULL);
    assert(pr->base.device->type == WLR_INPUT_DEVICE_POINTER);

    // Do nothing for now
    return true;
}

/*
 * Configure the input device for "input". This will attach listeners and device
 * specific logic. Returns false on failure.
 */
static bool
configure_input_device(struct viw_input *input)
{
    assert(input != NULL);

    switch (input->device->type)
    {
    case WLR_INPUT_DEVICE_KEYBOARD:
        return configure_keyboard((struct viw_keyboard *)input);
    case WLR_INPUT_DEVICE_POINTER:
        return configure_pointer((struct viw_pointer *)input);
    default:
        viw_error("Unknown input device \"%s\"", input->device->name);
        abort();
    }
}

static void
event_new_input(struct wl_listener *listener, void *data)
{
    struct viw_input_manager *imgr = wl_container_of(listener, imgr, new_input);
    struct wlr_input_device  *device = data;

    assert(imgr != NULL);

    size_t sz;

    switch (device->type)
    {
    case WLR_INPUT_DEVICE_KEYBOARD:
        sz = sizeof(struct viw_keyboard);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        sz = sizeof(struct viw_pointer);
        break;
    default:
        viw_warn("Unhandled input device \"%s\"", device->name);
        return;
    }

    struct viw_input *input = viw_calloc(1, sz);

    if (input == NULL)
        return;

    input->identifier = input_device_get_identifier(device);
    if (input->identifier == NULL)
    {
        viw_free(input);
        return;
    }

    input->device = device;
    input->viw = imgr->viw;

    if (!configure_input_device(input))
    {
        viw_error("Error configuring input device \"%s\"", input->identifier);
        viw_free(input->identifier);
        viw_free(input);
        return;
    }

    input->destroy.notify = input_destroy;
    wl_signal_add(&input->device->events.destroy, &input->destroy);

    viw_debug("New input device \"%s\"", input->identifier);

    wl_list_insert(&imgr->inputs, &input->link);
}

void
viw_input_manager_init(struct viw *viw, struct viw_input_manager *imgr)
{
    assert(imgr != NULL);

    wl_list_init(&imgr->inputs);
    wl_list_init(&imgr->seats);

    imgr->new_input.notify = event_new_input;
    wl_signal_add(&viw->backend->events.new_input, &imgr->new_input);

    viw_debug("Initialized input manager");
    imgr->viw = viw;
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
