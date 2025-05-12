/* save.c: Save file handler
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
#include <stdlib.h>
#include <string.h>

#include "bg.h"
#include "buffer.h"
#include "constants.h"
#include "demo.h"
#include "file.h"
#include "game.h"
#include "item.h"
#include "joy.h"
#include "lucia.h"
#include "mainmenu.h"
#include "platform.h"
#include "save.h"
#include "sound.h"
#include "sprite.h"
#include "system.h"
#include "task.h"
#include "util.h"
#include "weapon.h"

static Uint8 savePalette[] = {
    0x0f, 0x20, 0x20, 0x20,
    0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f,
    0x0F, 0x12, 0x16, 0x36,
    0x0F, 0x36, 0x26, 0x16,
    0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f,
};

#define NUM_FILES 3
static Buffer *files[NUM_FILES];
static int currFile;

void Save_Serialize(Buffer *buf) {
    Buffer_AddSint16(buf, maxHealth);
    Buffer_AddSint16(buf, maxMagic);
    Buffer_Add(buf, highestReachedStage);
    Buffer_Add(buf, keywordDisplay);
    Buffer_Add(buf, orbCollected);
    for (int i = 0; i < NUM_WEAPONS; i++) {
        Buffer_Add(buf, weaponLevels[i]);
    }
    Buffer_Add(buf, bootsLevel);
    for (int i = 0; i < ARRAY_LEN(itemsCollected); i++) {
        Buffer_Add(buf, itemsCollected[i]);
    }
    for (int i = 0; i < ARRAY_LEN(bossDefeated); i++) {
        Buffer_Add(buf, bossDefeated[i]);
    }
    // at the end because this is a late addition and I want to keep compatibility with existing save files
    Buffer_AddSint16(buf, magic);
}

int Save_Deserialize(Buffer *buf) {
    int cursor = 0;
    maxHealth = Util_LoadSint16(buf->data + cursor);
    cursor += 2;
    maxMagic = Util_LoadSint16(buf->data + cursor);
    cursor += 2;
    highestReachedStage = buf->data[cursor++];
    keywordDisplay = buf->data[cursor++];
    orbCollected = buf->data[cursor++];
    memcpy(weaponLevels, buf->data + cursor, sizeof(weaponLevels));
    cursor += sizeof(weaponLevels);
    bootsLevel = buf->data[cursor++];
    memcpy(itemsCollected, buf->data + cursor, sizeof(itemsCollected));
    cursor += sizeof(itemsCollected);
    memcpy(bossDefeated, buf->data + cursor, sizeof(bossDefeated));
    cursor += sizeof(bossDefeated);
    if (buf->dataSize > cursor) {
        magic = Util_LoadSint16(buf->data + cursor);
        cursor += 2;
    }
    else {
        // default value
        magic = 1000;
    }
    return cursor;
}

void Save_SaveFile(void) {
    char filename[20];

    if (!files[currFile]) {
        files[currFile] = Buffer_Init(64);
    }
    files[currFile]->dataSize = 0;
    Save_Serialize(files[currFile]);
    snprintf(filename, sizeof(filename), "file%d.sav", currFile + 1);
    Buffer_WriteToFile(files[currFile], filename);
}

void Save_EraseFile(int num) {
    Buffer_Destroy(files[num]);
    files[num] = NULL;
    char filename[20];
    snprintf(filename, sizeof(filename), "file%d.sav", num + 1);
    File_Remove(filename);
}

void Save_Init(void) {
    char filename[20];
    for (int i = 0; i < NUM_FILES; i++) {
        snprintf(filename, sizeof(filename), "file%d.sav", i + 1);
        FILE *fp = File_Open(filename, "rb");
        if (fp) {
            files[i] = Buffer_InitFromFile(fp);
            fclose(fp);
        }
        else {
            files[i] = NULL;
        }
    }
}

static void Save_DrawLucia(Sint16 x, Sint16 y, Uint16 tile) {
    Sprite luciaSpr = { 0 };
    luciaSpr.x = x - BG_CENTERED_X;
    luciaSpr.y = y;
    luciaSpr.size = SPRITE_16X16;
    luciaSpr.tile = tile;
    luciaSpr.palette = 0;
    luciaSpr.mirror = H_MIRROR;
    Sprite_Draw(&luciaSpr, NULL);
    luciaSpr.tile += 2;
    luciaSpr.y += 16;
    Sprite_Draw(&luciaSpr, NULL);
}

static Uint16 luciaRunTiles[] = {
    0x4, 0x8, 0xc, 0x8,
};

// menu indices
#define ERASE_INDEX NUM_FILES
#define MAIN_MENU_INDEX (ERASE_INDEX + 1)

// position defines
#define HEADER_X 10
#define HEADER_Y 2

#define ERASE_X 10
#define ERASE_Y 23
#define ERASE_Y_PX (ERASE_Y * 8)
#define ERASE_SPACING 3
#define ERASE_SPACING_PX (ERASE_SPACING * 8)

#define TITLE_X 10
#define TITLE_Y (ERASE_Y + ERASE_SPACING)

#define SAVE_X 8
#define SAVE_Y 6
#define SAVE_Y_PX (SAVE_Y * 8)
#define SAVE_SPACING 6
#define SAVE_SPACING_PX (SAVE_SPACING * 8)

#define LUCIA_X_PX 48
#define LUCIA_Y_PX (SAVE_Y_PX - 16)

void Save_Screen(void) {
    int erase = 0;
    int cursor = currFile;
    int stages[NUM_FILES];
    int highestStages[NUM_FILES];
    Sprite cursorSpr = { 0 };

    Sound_Reset();
    BG_Clear();
    BG_Scroll(BG_CENTERED_X, 0);
    BG_SetAllPalettes(savePalette);
    Sprite_SetAllPalettes(savePalette + 16);
    for (int i = 0; i < NUM_FILES; i++) {
        if (files[i]) {
            Save_Deserialize(files[i]);
            stages[i] = highestReachedStage;
            highestStages[i] = stages[i];
        }
    }

    while (1) {
        Sprite_ClearList();

        // cursor movement (with wraparound)
        if (joyEdge & JOY_UP) {
            cursor--;
            if (cursor < 0) { cursor = MAIN_MENU_INDEX; }
            Sound_Play(SFX_MENU);
        }
        if (joyEdge & (JOY_DOWN | JOY_SELECT)) {
            cursor++;
            if (cursor > MAIN_MENU_INDEX) { cursor = 0; }
            Sound_Play(SFX_MENU);
        }

        if (!erase) {
            cursorSpr.size = SPRITE_8X16;
            cursorSpr.tile = 0xee;
            cursorSpr.palette = 1;
            cursorSpr.mirror = 0;
        }
        else {
            cursorSpr.size = SPRITE_8X16;
            cursorSpr.tile = 0x48;
            cursorSpr.palette = 0;
            cursorSpr.mirror = H_MIRROR;
        }
        // first 3 positions = save files, last = "erase game" text
        if (cursor < NUM_FILES) {
            if (files[cursor] && !erase) {
                if (joyEdge & JOY_LEFT) {
                    stages[cursor]--;
                    stages[cursor] = MAX(stages[cursor], 0);
                    Sound_Play(SFX_MENU);
                }
                if (joyEdge & JOY_RIGHT) {
                    stages[cursor]++;
                    stages[cursor] = MIN(stages[cursor], highestStages[cursor]);
                    Sound_Play(SFX_MENU);
                }
            }
            // move the cursor over if there's a file
            cursorSpr.x = (files[cursor] ? 30 : 80) - BG_CENTERED_X;
            cursorSpr.y = (SAVE_Y_PX - 5) + (cursor * SAVE_SPACING_PX);
        }
        else {
            cursorSpr.x = 72 - BG_CENTERED_X;
            cursorSpr.y = (ERASE_Y_PX - 5) + ((cursor - NUM_FILES) * ERASE_SPACING_PX);
        }
        Sprite_Draw(&cursorSpr, NULL);

        // draw the text
        if (!erase) {
            BG_Print(HEADER_X, HEADER_Y, 0, "Select Game");
            BG_Print(ERASE_X,  ERASE_Y,  0, "Erase File");
            BG_Print(TITLE_X,  TITLE_Y,  0, "Main Menu");
        }
        else {
            BG_Print(HEADER_X, HEADER_Y, 0, "Erase File ");
            BG_Print(ERASE_X,  ERASE_Y,  0, "Cancel    ");
            BG_Print(TITLE_X,  TITLE_Y,  0, "Main Menu");
        }

        // draw save file text
        for (int i = 0; i < NUM_FILES; i++) {
            if (files[i]) {
                BG_Print(SAVE_X, (i * SAVE_SPACING) + SAVE_Y, 0, "File %d   Stage - %2u -", i + 1, stages[i] + 1);
                // if the save file exists, draw a Lucia sprite for it
                Save_DrawLucia(LUCIA_X_PX, (i * SAVE_SPACING_PX) + LUCIA_Y_PX, 0);
            }
            else {
                BG_Print(SAVE_X, (i * SAVE_SPACING) + SAVE_Y, 0, "   New Game            ");
            }
        }

        BG_Display();
        Sprite_Display();
        Task_Yield();

        if (joyEdge & (JOY_A | JOY_START)) {
            // toggle normal/erase mode
            if (cursor == ERASE_INDEX) {
                erase ^= 1;
                cursor = 0;
            }
            // return to main menu
            else if (cursor == MAIN_MENU_INDEX) {
                Task_Switch(MainMenu_Run);
            }
            // selected a file to erase
            else if (erase) {
                Save_EraseFile(cursor);
                erase = 0;
                cursor = 0;
            }
            // selected a file to play
            else { break; }
        }
    }

    currFile = cursor;
    if (files[cursor]) {
        // lucia running offscreen animation
        Sint16 luciaX = LUCIA_X_PX;
        int frames = 0;
        while (luciaX < 256) {
            Sprite_ClearList();
            Sprite_Draw(&cursorSpr, NULL);
            frames++;
            for (int i = 0; i < NUM_FILES; i++) {
                if (i == currFile) {
                    Save_DrawLucia(luciaX, (i * SAVE_SPACING_PX) + LUCIA_Y_PX, luciaRunTiles[(frames >> 3) & 3]);
                }
                else if (files[i]) {
                    Save_DrawLucia(LUCIA_X_PX, (i * SAVE_SPACING_PX) + LUCIA_Y_PX, 0);
                }
            }
            luciaX += 3;
            BG_Display();
            Sprite_Display();
            Task_Yield();
            if (joyEdge & JOY_START) { break; }
        }

        Save_Deserialize(files[cursor]);
        stage = stages[cursor];
        Task_Switch(Game_LoadGame);
    }
    Task_Switch(Game_NewGame);
}
