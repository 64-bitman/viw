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

#include "input.h" // IWYU pragma: keep
#include <wayland-server.h>
#include <wlr/types/wlr_seat.h>

struct viw_seat_input
{
    struct viw_input *input;
    struct wl_list    link;
};

struct viw_seat
{
    struct wlr_seat *global;

    struct wl_list inputs; // struct viw_seat_input
};
