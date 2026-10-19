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

#include "log.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// Should only be set at program startup
static enum viw_log_level LOG_LEVEL = VIW_LOG_INFO;

static const char *LOG_STR[] = {
    [VIW_LOG_DEBUG] = "debug",
    [VIW_LOG_INFO] = "info",
    [VIW_LOG_WARN] = "warn",
    [VIW_LOG_ERROR] = "error",
};

/*
 * Set minimum log level to output
 */
void
viw_log_set(enum viw_log_level level)
{
    LOG_LEVEL = level;
}

/*
 * Should be thread safe...
 */
void
viw_log_level(
    enum viw_log_level level, const char *file, int line, const char *fmt, ...
)
{
    assert(fmt != NULL);

    if (level < LOG_LEVEL)
        return;

    flockfile(stderr);
    fprintf(stderr, "[%s] (%s:%d) -> ", LOG_STR[level], file, line);

    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);

    funlockfile(stderr);
}
