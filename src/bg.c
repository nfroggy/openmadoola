/* bg.c: Background display code
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

#include <stdio.h>
#include <string.h>
#include "bg.h"
#include "constants.h"
#include "graphics.h"
#include "map.h"
#include "palette.h"

#define TEXT_BASE (0x800)

typedef struct {
    Uint16 tile;
    Uint8 palnum;
} BgTile;

static BgTile bgTiles[BG_HEIGHT][BG_WIDTH];
static Uint32 xScroll, yScroll;

void BG_Fill(Uint16 tile, Uint8 palnum) {
    for (int y = 0; y < BG_HEIGHT; y++) {
        for (int x = 0; x < BG_WIDTH; x++) {
            bgTiles[y][x].tile = tile;
            bgTiles[y][x].palnum = palnum;
        }
    }
}

void BG_Clear(void) {
    BG_Fill(0x7fc, 0);
}

void BG_ClearRow(Uint16 row) {
    if (row < BG_HEIGHT) {
        for (int i = 0; i < BG_WIDTH; i++) {
            bgTiles[row][i].tile = 0x7fc;
            bgTiles[row][i].palnum = 0;
        }
    }
}

void BG_SetTile(Uint16 x, Uint16 y, Uint8 palnum, Uint16 tile) {
    if ((x < BG_WIDTH) && (y < BG_HEIGHT)) {
        bgTiles[y][x].tile = tile;
        bgTiles[y][x].palnum = palnum;
    }
}

void BG_PutChar(Uint16 x, Uint16 y, Uint8 palnum, char ch) {
    BG_SetTile(x, y, palnum, ch + TEXT_BASE);
}

int BG_Print(Uint16 x, Uint16 y, Uint8 palnum, char *fmt, ...) {
    va_list args;
    // write formatted string to the buffer
    va_start(args, fmt);
    int len = BG_VPrint(x, y, palnum, fmt, args);
    va_end(args);
    return len;
}

int BG_VPrint(Uint16 x, Uint16 y, Uint8 palnum, char *fmt, va_list args) {
    char buff[512];
    int len = vsnprintf(buff, sizeof(buff), fmt, args);
    int startX = x;

    // copy buffer to the background
    char *text = buff;
    while (*text) {
        if (*text == '\n') {
            x = startX;
            y++;
        }
        else {
            BG_SetTile(x++, y, palnum, *text + TEXT_BASE);
        }
        text++;
    }
    return len;
}

void BG_SetPalette(int palnum, const Uint8 *palette) {
    memcpy(&colorPalette[palnum * PALETTE_SIZE], palette, PALETTE_SIZE);
}

void BG_SetAllPalettes(const Uint8 *palette) {
    memcpy(colorPalette, palette, PALETTE_SIZE * 4);
}

void BG_Scroll(Uint32 x, Uint32 y) {
    xScroll = x;
    yScroll = y;
}

void BG_Display(void) {
    for (int y = 0; y < SCREEN_HEIGHT + TILE_HEIGHT; y += TILE_HEIGHT) {
        for (int x = 0; x < SCREEN_WIDTH + TILE_WIDTH; x += TILE_WIDTH) {
            int xPos = x - (xScroll % TILE_WIDTH);
            int yPos = y - (yScroll % TILE_HEIGHT);

            int xTile = (((x + xScroll) / TILE_WIDTH) % BG_WIDTH);
            int yTile = (((y + yScroll) / TILE_HEIGHT) % BG_HEIGHT);

            Graphics_DrawTile(xPos, yPos, bgTiles[yTile][xTile].tile, bgTiles[yTile][xTile].palnum, 0);
        }
    }
}