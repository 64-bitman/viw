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
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>

struct viw;
struct viw_output_manager;

struct viw_output
{
    struct viw_output_manager *mgr;
    struct wlr_output         *output;

    struct wl_listener frame;
    struct wl_listener request_state;
    struct wl_listener destroy;

    struct wl_list link;

    struct
    {
        struct wl_signal destroy;
    } events;
};

struct viw_output_manager
{
    struct viw *viw;

    struct wl_list     outputs;
    struct wl_listener new_output; // struct wlr_output

    struct wlr_output_layout *output_layout;
};

// clang-format off
bool viw_output_manager_init(struct viw_output_manager *omgr, struct viw *viw);
void viw_output_manager_uninit(struct viw_output_manager *omgr);
// clang-format on
