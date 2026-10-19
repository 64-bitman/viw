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

#include <pthread.h>
#include <wayland-server.h>

struct viw_wakeup_callback;
typedef void (*viw_wakeup_func_T)(struct viw_wakeup_callback *cb);
struct viw_wakeup_callback
{
    viw_wakeup_func_T func;
    void             *udata;
    struct wl_list    link;
};

/*
 * Allows async callbacks to be called in another thread
 */
struct viw_wakeup
{
    bool init;

    struct wl_event_source *event_source;
    int                     event_fd;

    pthread_mutex_t mut;
    struct wl_list  pending;
};

// clang-format off
bool viw_wakeup_init(struct viw_wakeup *wk, struct wl_event_loop *event_loop);
void viw_wakeup_uninit(struct viw_wakeup *wk);
void viw_wakeup_flush(struct viw_wakeup *wk);
bool viw_wakeup_wakeup(struct viw_wakeup *wk);
void viw_wakeup_send(struct viw_wakeup *wk, struct viw_wakeup_callback *cb, viw_wakeup_func_T func);
// clang-format on
