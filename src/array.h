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

#include "mem.h" // IWYU pragma: keep
#include "misc.h"
#include <stdbool.h>

/*
 * Define an anonymous struct for dynamic array
 */
#define ARRAY_DEF(name, type)                                                  \
    struct                                                                     \
    {                                                                          \
        type  *arr;                                                            \
        size_t len;                                                            \
        size_t sz;                                                             \
    } name

#define array_init(s)                                                          \
    do                                                                         \
    {                                                                          \
        (s)->arr = NULL;                                                       \
        (s)->len = (s)->sz = 0;                                                \
    } while (false)
#define array_clear(s)                                                         \
    do                                                                         \
    {                                                                          \
        viw_free((s)->arr);                                                    \
        array_init(s);                                                         \
    } while (false)

/*
 * Grow the array if needed so that it can hold "n" elements
 */
#define array_grow(s, n)                                                       \
    do                                                                         \
    {                                                                          \
        if ((s)->len + n <= (s)->sz)                                           \
            break;                                                             \
        size_t new_sz = MAX((s)->len + n, (s)->sz * 2);                        \
        void *new = viw_realloc((s)->arr, new_sz * sizeof(*(s)->arr));         \
        if (new == NULL)                                                       \
            break;                                                             \
        (s)->arr = new;                                                        \
        (s)->sz = new_sz;                                                      \
    } while (false)

#define array_len(s) ((s)->len)
#define array_can(s, n)                                                        \
    ((s)->len + (n) <= (s)->sz) // true if array can store
                                // "n" more items, should be called after
                                // array_grow().
#define array_val(s, i) ((s)->arr[i])
#define array_ptr(s, i) ((s)->arr + (i))
#define array_add(s, n) ((s)->len += n)
#define array_del(s, n) ((s)->len -= n)
#define array_append(s, v) ((s)->arr[(s)->len++] = (v))
