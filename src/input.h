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

#pragma once

#include <wayland-server.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_pointer.h>

struct viw;

enum viw_input_event_type
{
    VIW_KEYBOARD_KEY,
    VIW_KEYBOARD_MODIFIERS,
};

struct viw_input_event
{
    enum viw_input_event_type type;

    union
    {
        struct wlr_keyboard_key_event *key;
    };
};

/*
 * Base type for input device
 */
struct viw_input
{
    struct viw *viw;

    char                    *identifier;
    struct wlr_input_device *device;
    struct wl_listener       destroy;
    struct wl_list           link;

    struct
    {
        struct wl_signal new_event;     // struct viw_input_event
        struct wl_signal input_destroy; // Called for struct viw_input
    } events;
};

/*
 * WLR_INPUT_DEVICE_KEYBOARD
 */
struct viw_keyboard
{
    struct viw_input base;

    struct wlr_keyboard *keyboard;

    struct wl_listener key;
    struct wl_listener modifiers;
};

/*
 * WLR_INPUT_DEVICE_POINTER
 */
struct viw_pointer
{
    struct viw_input base;

    struct wlr_pointer *pointer;
};

struct viw_input_manager
{
    bool        init;
    struct viw *viw;

    struct wl_list inputs; // struct viw_input
    struct wl_list seats;  // struct viw_seat;

    struct wl_listener new_input; // struct wlr_input_device

    struct
    {
        struct wl_signal input_added; // struct viw_input
    } events;
};

// clang-format off
void viw_input_manager_init(struct viw_input_manager *imgr, struct viw *viw);
void viw_input_manager_uninit(struct viw_input_manager *imgr);
// clang-format on
