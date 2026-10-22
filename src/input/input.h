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

    union
    {
        struct wlr_keyboard *keyboard;
        struct wlr_pointer  *pointer;
    } handle;

    struct
    {
        struct wl_signal destroy;
    } events;
};

struct viw_input_manager
{
    struct viw *viw;

    struct wl_list inputs; // struct viw_input
    struct wl_list seats;  // struct viw_seat;

    struct wl_listener new_input; // struct wlr_input_device

    struct
    {
        struct wl_signal new_input; // struct viw_input
    } events;
};

// clang-format off
void viw_input_manager_init(struct viw_input_manager *imgr, struct viw *viw);
void viw_input_manager_uninit(struct viw_input_manager *imgr);
// clang-format on
