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

#include "event.h"
#include "log.h"
#include "misc.h"
#include <assert.h>
#include <sys/eventfd.h>
#include <unistd.h>

static bool
trigger_callbacks(struct viw_wakeup *wk, int64_t limit_ns)
{
    assert(wk != NULL);

    int64_t start;
    bool    timed_out = false;

    struct viw_wakeup_callback *cb, *cb_tmp;

    if (limit_ns > 0)
        start = get_time_ns(CLOCK_MONOTONIC);

    pthread_mutex_lock(&wk->mut);
    wl_list_for_each_safe(cb, cb_tmp, &wk->pending, link)
    {
        if (limit_ns > 0 && get_time_ns(CLOCK_MONOTONIC) - start > limit_ns)
        {
            viw_info("Exceeded time limit for async callbacks");
            timed_out = true;
            break;
        }

        wl_list_remove(&cb->link);
        cb->func(cb);
    }
    pthread_mutex_unlock(&wk->mut);
    return timed_out;
}

static int
wakeup_handler(int fd, uint32_t revents, void *udata)
{
    if (!(revents & WL_EVENT_READABLE))
        return 0;

    int64_t v;

    if (read(fd, &v, sizeof(v)) == -1)
        viw_warn_errno("Error reading eventfd for wakeup");

    struct viw_wakeup *wk = udata;

    // Allow at most 2 milliseconds for handling callbacks
    return trigger_callbacks(wk, 2000000) ? 1 : 0;
}

bool
viw_wakeup_init(struct viw_wakeup *wk, struct wl_event_loop *event_loop)
{
    assert(wk != NULL);
    assert(event_loop != NULL);

    memset(wk, 0, sizeof(*wk));

    wk->event_fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (wk->event_fd == -1)
    {
        viw_error_errno("Error creating eventfd for wakeup source");
        return false;
    }

    wk->event_source = wl_event_loop_add_fd(
        event_loop, wk->event_fd, WL_EVENT_READABLE, wakeup_handler, wk
    );
    if (wk->event_source == NULL)
    {
        viw_error_errno("Error creating wakeup source");
        close(wk->event_fd);
        return false;
    }

    pthread_mutex_init(&wk->mut, NULL);
    wl_list_init(&wk->pending);
    wk->init = true;

    return true;
}

/*
 * Will trigger all pending callbacks if any, so should be called before freeing
 * anything.
 */
void
viw_wakeup_uninit(struct viw_wakeup *wk)
{
    assert(wk != NULL);

    if (!wk->init)
        return;

    (void)trigger_callbacks(wk, 0);

    wl_event_source_remove(wk->event_source);
    close(wk->event_fd);

    pthread_mutex_destroy(&wk->mut);
}

/*
 * Trigger any pending callbacks
 */
void
viw_wakeup_flush(struct viw_wakeup *wk)
{
    assert(wk != NULL);
    trigger_callbacks(wk, 0);
}

/*
 * Write to the eventfd so that the event loop is woken up.
 */
bool
viw_wakeup_wakeup(struct viw_wakeup *wk)
{
    int64_t v = 1;
    if (write(wk->event_fd, &v, sizeof(v)) == -1)
    {
        viw_warn_errno("Error waking up event loop");
        return false;
    }
    return true;
}

/*
 * Wakeup event loop and queue an asynchronous callback
 */
void
viw_wakeup_send(
    struct viw_wakeup          *wk,
    struct viw_wakeup_callback *cb, // Does not have to be initialized
    viw_wakeup_func_T           func
)
{
    assert(wk != NULL);
    assert(cb != NULL);
    assert(func != NULL);

    cb->func = func;

    pthread_mutex_lock(&wk->mut);
    wl_list_insert(wk->pending.prev, &cb->link);
    pthread_mutex_unlock(&wk->mut);

    (void)viw_wakeup_wakeup(wk);
}
