/* sprite.h: Sprite display code
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
#include "constants.h"
#include "graphics.h"
#include "object.h"

typedef enum {
    SPRITE_NONE = 0,
    SPRITE_8X8,
    SPRITE_8X16,
    SPRITE_16X16,
} SPRITE_SIZE;

typedef struct {
    Sint16 x; // in screen-space pixels
    Sint16 y; // in screen-space pixels
    Uint8 size; // from SPRITE_SIZE enum
    Uint16 tile; // tile number
    Uint8 palette; // palette number
    Uint8 mirror; // V_MIRROR, H_MIRROR, or both 
} Sprite;

/**
 * @brief Sets a sprite palette
 * @param palnum the palette number (0-7)
 * @param palette 4-byte palette array
*/
void Sprite_SetPalette(int palnum, const Uint8 *palette);

/**
 * @brief Sets sprite palettes 0-4
 * @param palette 16-byte palette array
*/
void Sprite_SetAllPalettes(const Uint8 *palette);

/**
 * @brief Clears the sprite list, should be run at the start of each frame
*/
void Sprite_ClearList(void);

/**
 * @brief Clears the overlay sprite list
 */
void Sprite_ClearOverlayList(void);

/**
 * @brief Gets the next free sprite in the sprite list.
 * Note that this will get overwritten every frame unless you don't call Sprite_ClearList.
 * @returns a pointer to the next free sprite in the sprite list, or NULL if there aren't any free sprites.
*/
Sprite *Sprite_Get(void);

/**
 * @brief Sets up screen space x/y coordinates for the given sprite
 * @param s The sprite to draw
 * @param o The object that's drawing the sprite
 * @param xOffset pixel value to be added to the x display position
 * @param yOffset pixel value to be added to the y display position
 * @returns zero if the sprite is offscreen, nonzero otherwise
*/
int Sprite_SetPos(Sprite *s, Object *o, Sint16 xOffset, Sint16 yOffset);

/**
 * @brief Adds a sprite to the list to be drawn
 * @param s The sprite to add
 * @param o The object that's drawing the sprite
*/
void Sprite_Draw(Sprite *s, Object *o);

/**
 * @brief like Sprite_Draw but sets the mirror flag in the sprite struct from
 * the direction in the object
 * @param s The sprite to draw
 * @param o The object that's drawing the sprite
*/
void Sprite_DrawDir(Sprite *s, Object *o);

/**
 * @brief Like calling Sprite_SetPos and then Sprite_DrawDir
 * @param s The sprite to draw
 * @param o The object that's drawing the sprite
 * @param xOffset pixel value to be added to the x display position
 * @param yOffset pixel value to be added to the y display position
 * @returns zero if the sprite is offscreen, nonzero otherwise
*/
int Sprite_SetDraw(Sprite *s, Object *o, Sint16 xOffset, Sint16 yOffset);

/**
 * @brief Helper function for using Sprite_SetDraw on a 16x32 sprite.
 * @param s Sprite to draw. Tile should be set to the bottom 16x16 tile.
 * @param o The object that's drawing the sprite. Gets set to OBJ_NONE if the sprite is offscreen.
 * @param topTile The upper 16x16 tile to draw.
 * @param xOffset pixel value to be added to the x display position
 * @param yOffset pixel value to be added to the y display position
 * @returns zero if the sprite is offscreen, nonzero otherwise
*/
int Sprite_SetDraw16x32(Sprite *s, Object *o, Uint16 topTile, Sint16 xOffset, Sint16 yOffset);

/**
 * @brief Helper function for drawing a large metasprite.
 * @param s Sprite to draw (only palette and mirror get read)
 * @param o The object that's drawing the sprite
 * @param tiles Array of tile numbers, terminated by 0
 * @param offsets Array of tile offsets, in x, y order
 * @param xOffset pixel value to be added to the x display position
 * @param yOffset pixel value to be added to the y display position
 * @returns zero if the sprite is offscreen, nonzero otherwise
*/
int Sprite_SetDrawLarge(Sprite *s, Object *o, Uint16 *tiles, Sint8 *offsets, Sint16 xOffset, Sint16 yOffset);

/**
 * @brief Like Sprite_SetDrawLarge but doesn't initialize the sprite's position
 * @param s Sprite to draw
 * @param o Object that's drawing the sprite
 * @param tiles Array of tile numbers, terminated by 0
 * @param offsets Array of tile offsets, in x, y order
*/
void Sprite_SetDrawLargeAbs(Sprite *s, Object *o, Uint16 *tiles, Sint8 *offsets);

/**
 * @brief Adds a sprite to the list to be drawn
 * @param s Sprite to add to the list
 */
void Sprite_DrawOverlay(Sprite *s);

/**
 * @brief Draws all the sprites in the sprite list, should be run at the end of each frame
*/
void Sprite_Display(void);

/**
 * @brief Renders all the sprites in the overlay list to the screen.
 */
void Sprite_DisplayOverlay(void);
