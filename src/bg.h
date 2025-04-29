/* bg.h: Background display code
 * Copyright (c) 2023-2025 Nathan Misner
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
#include <stdarg.h>
#include "constants.h"
#include "map.h"

#define BG_WIDTH (64)
#define BG_HEIGHT (64)
#define BG_CENTERED_X ((256 / 2) - (SCREEN_WIDTH / 2))

/**
 * @brief Sets all background tiles to the same tile and palette number.
 * @param tile the tile number
 * @param palnum the palette number
*/
void BG_Fill(Uint16 tile, Uint8 palnum);

/**
 * @brief Sets all background tiles to 0x7fc (empty tile)
 */
void BG_Clear(void);

/**
 * @brief Sets all background tiles in the given row to 0x7fc (empty tile)
 * @param row the rown number to clear
 */
void BG_ClearRow(Uint16 row);

/**
 * @brief Sets a tile's value on the tilemap
 * @param x the tile's x position
 * @param y the tile's y position
 * @param palnum the palette number
 * @param tilenum the tile's number
*/
void BG_SetTile(Uint16 x, Uint16 y, Uint8 palnum, Uint16 tile);

/**
 * @brief Puts a character on the background
 * @param x character x position
 * @param y character y position
 * @param palnum character palette number
 * @param ch character to put
 */
void BG_PutChar(Uint16 x, Uint16 y, Uint8 palnum, char ch);

/**
 * @brief Prints a message to the screen
 * @param x x position
 * @param y y position
 * @param palnum palette number
 * @param fmt text to print
 * @param ... arguments
 * @returns The length of the printed message
 */
#ifdef __GNUC__
__attribute__((__format__(__printf__, 4, 5)))
#endif
int BG_Print(Uint16 x, Uint16 y, Uint8 palnum, char *fmt, ...);

/**
 * @brief Prints a message to the screen
 * @param x x position
 * @param y y position
 * @param palnum palette number
 * @param fmt text to print
 * @param args arguments
 * @returns The length of the printed message
 */
#ifdef __GNUC__
__attribute__((__format__(__printf__, 4, 0)))
#endif
int BG_VPrint(Uint16 x, Uint16 y, Uint8 palnum, char *fmt, va_list args);

/**
 * @brief Sets a background palette
 * @param palnum the palette number to set (0-3)
 * @param palette the 4-byte palette array to write
*/
void BG_SetPalette(int palnum, const Uint8 *palette);

/**
 * @brief Sets all background palettes
 * @param palettes the 16-byte palette array to write
*/
void BG_SetAllPalettes(const Uint8 *palettes);

/**
 * @brief Sets the background's scroll position.
 * @param x The background x position
 * @param y The background y position
*/
void BG_Scroll(Uint32 x, Uint32 y);

/**
 * @brief Draws the background to the screen. Should be run once per frame
*/
void BG_Display(void);
