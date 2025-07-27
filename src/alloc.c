/* alloc.c: Memory allocation wrapper code
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

#include <stdlib.h>

void *ommalloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        abort();
    }
    return ptr;
}

void *omaligned_alloc(size_t alignment, size_t size) {
    if (size % alignment) {
        abort();
    }
    void *ptr = aligned_alloc(alignment, size);
    if (!ptr) {
        abort();
    }
    return ptr;
}

void *omrealloc(void *ptr, size_t new_size) {
    ptr = realloc(ptr, new_size);
    if (!ptr) {
        abort();
    }
    return ptr;
}
