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

#include "mem.h"
#include <stdlib.h>
#include <string.h>

void *
viw_malloc(size_t sz)
{
    return malloc(sz);
}

void *
viw_calloc(size_t n, size_t sz)
{
    return calloc(n, sz);
}

void *
viw_realloc(void *ptr, size_t sz)
{
    return realloc(ptr, sz);
}

void
viw_free(void *ptr)
{
    free(ptr);
}

char *
viw_strdup(const char *str)
{
    return strdup(str);
}

char *
viw_strndup(const char *str, size_t len)
{
    return strndup(str, len);
}
