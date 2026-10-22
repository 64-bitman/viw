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
#include <wlr/types/wlr_cursor.h>

struct viw_seat;

struct viw_cursor
{
    struct viw_seat   *seat;
    struct wlr_cursor *cursor;
};

// clang-format off
struct viw_cursor *viw_cursor_create(struct viw_seat *seat);
void viw_cursor_destroy(struct viw_cursor *csr);
// clang-format on
