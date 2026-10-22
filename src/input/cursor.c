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

#include "cursor.h"
#include "log.h"
#include "mem.h"
#include "seat.h"
#include <assert.h>

/*
 * Create a new cursor object. Returns NULL on failure.
 */
struct viw_cursor *
viw_cursor_create(struct viw_seat *seat)
{
    assert(seat != NULL);

    struct viw_cursor *csr = viw_calloc(1, sizeof(*csr));

    if (csr == NULL)
        return NULL;

    csr->cursor = wlr_cursor_create();
    if (csr->cursor == NULL)
    {
        viw_error("Error creating cursor");

        viw_free(csr);
        return NULL;
    }

    csr->seat = seat;

    return csr;
}

void
viw_cursor_destroy(struct viw_cursor *csr)
{
    assert(csr != NULL);

    wlr_cursor_destroy(csr->cursor);

    viw_free(csr);
}
