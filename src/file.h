/* file.h: File management utility functions
 * Copyright (c) 2023, 2024 Nathan Misner
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
#include <stdio.h>
#include "constants.h"

/**
 * @brief Writes a 16-bit number to a file as big endian data.
 * @param data The number to write
 * @param fp The file to write to
 */
void File_WriteUint16BE(Uint16 data, FILE *fp);

/**
 * @brief Reads a 16-bit big endian number from a file.
 * @param fp The file to read from
 * @returns The read number
 */
Uint16 File_ReadUint16BE(FILE *fp);

/**
 * @brief Writes a 32-bit number to a file as big endian data.
 * @param data The number to write
 * @param fp The file to write to
 */
void File_WriteUint32BE(Uint32 data, FILE *fp);

/**
 * @brief Reads a 32-bit big endian number from a file.
 * @param fp The file to read from
 * @returns The read number
 */
Uint32 File_ReadUint32BE(FILE *fp);

/**
 * @brief Opens the given file from $XDG_DATA_HOME/openmadoola/filename or
 * $HOME/.openmadoola/filename on unix-like systems, current working
 * directory on other systems.
 * @param filename the name of the file to open
 * @param mode fopen mode code
 * @returns the file pointer to the opened file
 */
FILE *File_Open(const char *filename, const char *mode);

/**
 * @brief Deletes the given file. Attempts to delete it from $XDG_DATA_HOME/openmadoola/filename,
 * $HOME/.openmadoola/filename, and current working directory/filename on unix-like systems, and
 * just current working directory on other systems.
 * @param filename name of the file to delete
 */
void File_Remove(const char *filename);

/**
 * @brief For opening a read-only data file. On unix-like systems, iterates
 * through a few directories before giving up (see resourceDirs array in file.c)
 * @param filename the name of the file to open
 * @param mode fopen mode code
 * @returns the file pointer to the opened file
 */
FILE *File_OpenResource(const char *filename, const char *mode);

/**
 * @brief Loads the contents of the given file to memory
 * @param fp The file to read 
 * @param size (out, optional) Tells the size of the provided file
 * @returns A pointer to the loaded data (malloced, user code must free it)
 */
Uint8 *File_Load(FILE *fp, int *size);
