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

#include "event.h"
#include <wayland-server.h>

struct viw_config_interface
{
    bool init;

    char *config_path; // Path to config directory, set on initialization once.

    // Config thread uses its own event loop
    struct wl_event_loop *event_loop; // Created on main thread and transferred
                                      // ownership of to config thread
    struct viw_wakeup cfg_wk;

    pthread_t       thread;
    bool            run;
    pthread_mutex_t run_mut;
};

// clang-format off
bool viw_config_interface_init(struct viw_config_interface *iface);
bool viw_config_interface_start(struct viw_config_interface *iface);
void viw_config_interface_uninit(struct viw_config_interface *iface);
// clang-format on
