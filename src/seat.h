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

#include "array.h"
#include <pcre2.h>
#include <wayland-server.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_seat.h>

struct viw;
struct viw_seat;
struct viw_cursor;
struct viw_seat_manager;

/*
 * Base type for handle to an input device for a seat
 */
struct viw_seat_input
{
    struct viw_seat  *seat;
    struct viw_input *input;

    struct wl_listener destroy;
    struct wl_list     link;
};

struct viw_seat_keyboard
{
    struct viw_seat_input base;
    struct wl_listener    key; // struct wlr_keyboard_key_event
    struct wl_listener    modifiers;
};

struct viw_seat
{
    struct viw_seat_manager *mgr;

    struct wlr_seat   *global;
    struct viw_cursor *csr;
    struct wl_list     inputs; // struct viw_seat_input

    // Array of patterns to match new input devices to this seat
    ARRAY_DEF(input_patterns, pcre2_code *);
    bool match_all; // If "all", then match every input device, "input_patterns"
                    // is ignorede.

    struct wl_list link;
};

struct viw_seat_manager
{
    struct viw *viw;

    struct wl_list seats;

    struct wl_listener input_added; // struct viw_input
};

// clang-format off
void viw_seat_manager_init(struct viw_seat_manager *smgr, struct viw *viw);
void viw_seat_manager_uninit(struct viw_seat_manager *smgr);
bool viw_seat_manager_default(struct viw_seat_manager *smgr);
struct viw_seat *viw_seat_add(struct viw_seat_manager *smgr, const char *name);
void viw_seat_remove(struct viw_seat *seat);
void viw_seat_add_input_pattern(struct viw_seat *seat, const char *pat);
// clang-format on
