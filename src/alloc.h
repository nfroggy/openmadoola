/* alloc.h: Memory allocation wrapper code
 * Copyright (c) 2024 Nathan Misner
 *
 * This file is part of OpenMadoola.
 *
 * OpenMadoola is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * OpenMadoola is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMadoola. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#include <stddef.h>

/**
 * @brief malloc wrapper that aborts on out of memory
 * @param size allocation size
 * @returns pointer to allocated memory
 */
void *ommalloc(size_t size);

/**
 * @brief aligned_alloc wrapper that aborts on out of memory or invalid alignment
 * @param alignment memory alignment
 * @param size allocation size
 * @returns pointer to allocated memory
 */
void *omaligned_alloc(size_t alignment, size_t size);

/**
 * @brief Workaround for Windows memory allocator stupidity. Needs to be called to
 * free memory allocated by omaligned_alloc, if you call regular free you'll get weird
 * bugs on Windows.
 * @param mem memory to free
 */
void omaligned_free(void *mem);

/**
 * @brief realloc wrapper that aborts on out of memory
 * @param ptr pointer to allocated buffer
 * @param new_size size to change buffer to
 * @returns pointer to allocated memory
 */
void *omrealloc(void *ptr, size_t new_size);
