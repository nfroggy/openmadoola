/* hud.c: HUD display code
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

#include "constants.h"
#include "hud.h"
#include "sprite.h"
#include "weapon.h"

// the tile number of the "0" tile
#define BASE_NUM 0x6C
#define BORDER_TILE 0x5C
#define DIGIT_WIDTH 8
#define HUD_WIDTH 48
#define WEAPON_WIDTH 16

/**
* @brief Displays the given 4-digit number.
* @param num The number to display
* @param xPos the x position to display the number at
* @param yPos the y position to display the number at
* @param palnum the sprite palette number to use
* @param borders if borders should be shown
*/
static void HUD_ShowNum(Sint16 num, Uint16 xPos, Uint16 yPos, Uint8 palnum, Uint8 borders) {
    Sprite spr = { 0 };
    spr.mirror = 0;
    spr.size = SPRITE_8X16;
    spr.x = xPos + (DIGIT_WIDTH * 4);
    spr.y = yPos;
    spr.palette = palnum;

    if (num > 9999) {
        num = 9999;
    }

    // draw number sprites
    for (int i = 0; i < 4; i++) {
        spr.tile = ((num % 10) * 2) + BASE_NUM;
        Sprite_DrawOverlay(&spr);
        spr.x -= DIGIT_WIDTH;
        num /= 10;
    }

    if (borders) {
        // draw left border
        spr.x = xPos;
        spr.tile = BORDER_TILE;
        Sprite_DrawOverlay(&spr);

        // draw right border
        spr.x = xPos + (5 * DIGIT_WIDTH);
        spr.mirror = H_MIRROR;
        Sprite_DrawOverlay(&spr);
    }
}

static void HUD_ShowScore(Uint32 num, Uint16 xPos, Uint16 yPos, Uint8 palnum) {
    if (num > 99999999) { num = 99999999; }
    HUD_ShowNum((Sint16)(num / 10000), xPos, yPos, palnum, 0);
    HUD_ShowNum((Sint16)(num % 10000), xPos + 32, yPos, palnum, 0);
}

void HUD_DisplayOriginal(Sint16 health, Sint16 magic) {
    HUD_ShowNum(health, (SCREEN_WIDTH / 2) - (HUD_WIDTH / 2), 16, 3, 1);
    HUD_ShowNum(magic, (SCREEN_WIDTH / 2) - (HUD_WIDTH / 2), SCREEN_HEIGHT - 32, 0, 1);
}

void HUD_DisplayPlus(Sint16 health, Sint16 magic) {
    HUD_ShowNum(health, 14, 16, 3, 1);
    HUD_ShowNum(magic, 14, 36, 0, 1);
    HUD_Weapon(18, 55);
}

void HUD_DisplayArcade(Sint16 health, Sint16 magic, Uint32 score) {
    HUD_ShowScore(score, 16, 23, 1);
    HUD_ShowNum(health, SCREEN_WIDTH - 60, SCREEN_HEIGHT - 65, 3, 0);
    HUD_ShowNum(magic, SCREEN_WIDTH - 60, SCREEN_HEIGHT - 41, 0, 0);
    HUD_Weapon(SCREEN_WIDTH - 68, SCREEN_HEIGHT - 41);
}

static Uint8 weaponTiles[] = {
    0x60, 0x60, 0x66, 0x62, 0x64, 0x68, 0x6a,
};

static Uint8 weaponPalettes[] = {
    1, 3, 3, 1, 3, 3, 1,
};

void HUD_Weapon(Sint16 x, Sint16 y) {
    Sprite spr = { 0 };

    // first weapon background
    spr.size = SPRITE_8X16;
    spr.x = x;
    spr.y = y;
    spr.tile = 0x4C;
    spr.mirror = 0;
    spr.palette = 0;
    Sprite_DrawOverlay(&spr);

    // second weapon background
    spr.x += 8;
    spr.mirror = H_MIRROR;
    Sprite_DrawOverlay(&spr);

    // weapon
    spr.x = x + 4;
    spr.tile = weaponTiles[currentWeapon];
    spr.mirror = 0;
    spr.palette = weaponPalettes[currentWeapon];
    Sprite_DrawOverlay(&spr);
}
