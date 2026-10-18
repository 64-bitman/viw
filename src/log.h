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

#include "misc.h"
#include <errno.h>  // IWYU pragma: keep
#include <string.h> // IWYU pragma: keep

enum viw_log_level
{
    VIW_LOG_DEBUG = 0,
    VIW_LOG_INFO,
    VIW_LOG_WARN,
    VIW_LOG_ERROR,
};

#define viw_log(level, fmt, ...)                                               \
    viw_log_level(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define viw_log_errno(level, fmt, ...)                                         \
    viw_log_level(                                                             \
        level, __FILE__, __LINE__, fmt ": %s", ##__VA_ARGS__, strerror(errno)  \
    )

#define viw_debug(fmt, ...) viw_log(VIW_LOG_DEBUG, fmt, ##__VA_ARGS__)
#define viw_info(fmt, ...) viw_log(VIW_LOG_INFO, fmt, ##__VA_ARGS__)
#define viw_warn(fmt, ...) viw_log(VIW_LOG_WARN, fmt, ##__VA_ARGS__)
#define viw_error(fmt, ...) viw_log(VIW_LOG_ERROR, fmt, ##__VA_ARGS__)

#define viw_debug_errno(fmt, ...)                                              \
    viw_log_errno(VIW_LOG_DEBUG, fmt, ##__VA_ARGS__)
#define viw_info_errno(fmt, ...) viw_log_errno(VIW_LOG_INFO, fmt, ##__VA_ARGS__)
#define viw_warn_errno(fmt, ...) viw_log_errno(VIW_LOG_WARN, fmt, ##__VA_ARGS__)
#define viw_error_errno(fmt, ...)                                              \
    viw_log_errno(VIW_LOG_ERROR, fmt, ##__VA_ARGS__)

// clang-format off
void viw_log_set(enum viw_log_level level);
void viw_log_level(enum viw_log_level level, const char *file, int line, const char *fmt, ...) PRINTFLIKE(4, 5);
// clang-format on
