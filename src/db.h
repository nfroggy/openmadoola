/* db.h: "database" handler
 * Copyright (c) 2023 Nathan Misner
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
#include "constants.h"

typedef struct {
    char *name;
    Uint32 dataLen;
    Uint8 *data;
} DBEntry;

/**
 * @brief Should be run on startup
 */
void DB_Init(void);

/**
 * @brief Finds the given db item
 * @param name item name to look for
 * @returns either a DBEntry pointer or NULL if it couldn't be found
 */
DBEntry *DB_Find(const char *name);

/**
 * @brief Saves the db to disk
 */
void DB_Save(void);

/**
 * @brief Sets the given db item to the given data, creating it if necessary
 * @param name item name
 * @param data data to write
 * @param dataLen size of data in bytes
 */
void DB_Set(const char *name, Uint8 *data, Uint32 dataLen);