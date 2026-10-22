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

#include "keyboard.h"
#include "log.h"
#include "seat.h"
#include <assert.h>
#include <wlr/types/wlr_keyboard.h>

/*
 * Configure the keyboard input device
 */
void
viw_input_configure_keyboard(struct viw_input *input)
{
    assert(input != NULL);

    struct xkb_context *ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    if (ctx == NULL)
    {
        viw_warn_errno("Error creating xkb context");
        return;
    }

    // TODO allow configuring of xkb keymap + use xkb file
    struct xkb_keymap *keymap = xkb_keymap_new_from_names2(
        ctx, NULL, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS
    );

    xkb_context_unref(ctx);

    if (keymap == NULL)
    {
        viw_warn_errno("Error creating xkb keymap");
        return;
    }

    wlr_keyboard_set_keymap(input->handle.keyboard, keymap);
    xkb_keymap_unref(keymap); // Above call takes reference

    wlr_keyboard_set_repeat_info(input->handle.keyboard, 30, 200); // TODO
}

static void
event_wlr_keyboard_key(struct wl_listener *listener, void *data)
{
    struct viw_seat_keyboard      *skb = wl_container_of(listener, skb, key);
    struct wlr_keyboard_key_event *event = data;

    (void)event;
}

static void
event_wlr_keyboard_modifiers(struct wl_listener *listener, void *data UNUSED)
{
    struct viw_seat_keyboard *skb = wl_container_of(listener, skb, modifiers);
}

void
viw_seat_keyboard_configure(struct viw_seat_keyboard *skb)
{
    assert(skb != NULL);

    struct wlr_keyboard *keyboard = skb->base.input->handle.keyboard;

    skb->key.notify = event_wlr_keyboard_key;
    wl_signal_add(&keyboard->events.key, &skb->key);

    skb->modifiers.notify = event_wlr_keyboard_modifiers;
    wl_signal_add(&keyboard->events.modifiers, &skb->modifiers);
}

void
viw_seat_keyboard_unconfigure(struct viw_seat_keyboard *skb)
{
    assert(skb != NULL);

    wl_list_remove(&skb->key.link);
    wl_list_remove(&skb->modifiers.link);
}
