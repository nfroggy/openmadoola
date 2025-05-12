/* pausemenu.c: Game pause menu
* Copyright (c) 2025 Nathan Misner
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
#include "game.h"
#include "joy.h"
#include "pausemenu.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"

#define TEXT_BASE (0x800)
static const Uint8 menuPalette[] = {
    0x0F, 0x16, 0x11, 0x31,
};

static const Uint8 arcadeMenuPalette[] = {
    0x0F, 0x06, 0x11, 0x31,
};

static const Uint8 selectorPalette[] = {
    0x0F, 0x36, 0x26, 0x16,
};

static int cursor;

void PauseMenu_Init(void) {
    Sprite_SetPalette(5, (gameType == GAME_TYPE_ARCADE) ? arcadeMenuPalette : menuPalette);
    Sprite_SetPalette(6, selectorPalette);
    cursor = 0;
}

static void PauseMenu_PrintStr(Sint16 x, Sint16 y, const char *text) {
    Sprite spr = { 0 };
    spr.x = x;
    spr.y = y;
    spr.palette = 5;
    spr.size = SPRITE_8X8;

    while (*text) {
        spr.tile = TEXT_BASE + *text++;
        Sprite_DrawOverlay(&spr);
        spr.x += 8;
    }
}

static void PauseMenu_DrawBG(Sint16 x, Sint16 y, Sint16 w, Sint16 h) {
    Sprite spr = { 0 };
    spr.palette = 5;
    spr.size = SPRITE_8X8;

    // only draw side borders if we're not in arcade mode to match the hud
    if (gameType != GAME_TYPE_ARCADE) {
        // top left corner
        spr.x = x - 8;
        spr.y = y;
        spr.tile = 0x880;
        Sprite_DrawOverlay(&spr);

        // top right corner
        spr.x = x + w * 8;
        spr.mirror = H_MIRROR;
        Sprite_DrawOverlay(&spr);

        // bottom right corner
        spr.y = y + (h - 1) * 8;
        spr.mirror = H_MIRROR | V_MIRROR;
        Sprite_DrawOverlay(&spr);

        // bottom left corner
        spr.x = x - 8;
        spr.mirror = V_MIRROR;
        Sprite_DrawOverlay(&spr);

        // left/right borders
        spr.tile = 0x881;
        for (Sint16 i = 1; i < (h - 1); i++) {
            spr.x = x - 8;
            spr.y = y + (i * 8);
            spr.mirror = 0;
            Sprite_DrawOverlay(&spr);
            spr.x = x + w * 8;
            spr.mirror = H_MIRROR;
            Sprite_DrawOverlay(&spr);
        }
    }

    // top/bottom borders
    spr.tile = 0x882;
    for (Sint16 i = 0; i < w; i++) {
        spr.x = x + (i * 8);
        spr.y = y;
        spr.mirror = 0;
        Sprite_DrawOverlay(&spr);
        spr.y = y + (h - 1) * 8;
        spr.mirror = V_MIRROR;
        Sprite_DrawOverlay(&spr);
    }

    // fill internals
    spr.tile = 0x7ff;
    spr.mirror = 0;
    for (Sint16 i = 1; i < (h - 1); i++) {
        for (Sint16 j = 0; j < w; j++) {
            spr.x = x + (j * 8);
            spr.y = y + (i * 8);
            Sprite_DrawOverlay(&spr);
        }
    }
}

#define PAUSE_MENU_WIDTH 8
#define PAUSE_MENU_HEIGHT 5
#define PAUSE_MENU_X_POS ((SCREEN_WIDTH / 2) - (PAUSE_MENU_WIDTH * TILE_WIDTH / 2))
#define PAUSE_MENU_Y_POS (80)

int PauseMenu_Run(void) {
    if (joyEdge & (JOY_UP | JOY_DOWN)) {
        cursor ^= 1;
        Sound_Play(SFX_MENU);
    }

    PauseMenu_DrawBG(PAUSE_MENU_X_POS, PAUSE_MENU_Y_POS, PAUSE_MENU_WIDTH, PAUSE_MENU_HEIGHT);
    PauseMenu_PrintStr(PAUSE_MENU_X_POS + 13, PAUSE_MENU_Y_POS + 8,
        (gameType == GAME_TYPE_ORIGINAL) ? "RESUME" : "Resume");
    PauseMenu_PrintStr(PAUSE_MENU_X_POS + 13, PAUSE_MENU_Y_POS + 24,
        (gameType == GAME_TYPE_ORIGINAL) ? "QUIT" : "Quit");

    Sprite spr = { 0 };
    spr.x = PAUSE_MENU_X_POS + 6;
    spr.y = PAUSE_MENU_Y_POS + (cursor ? 20 : 4);
    spr.palette = 6;
    spr.size = SPRITE_8X16;
    spr.tile = 0xee;
    Sprite_DrawOverlay(&spr);

    // always unpause if the user presses start
    if (joyEdge & JOY_START) {
        return PAUSE_EXIT_RESUME;
    }
    // if the user presses a, quit if the quit option is selected
    if (joyEdge & JOY_A) {
        // make sure the button press doesn't interfere with gameplay
        joyEdge &= ~JOY_A;
        return cursor ? PAUSE_EXIT_QUIT : PAUSE_EXIT_RESUME;
    }
    return PAUSE_EXIT_NONE;
}