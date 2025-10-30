/* options.c: Options screen
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

#include "bg.h"
#include "constants.h"
#include "db.h"
#include "game.h"
#include "graphics.h"
#include "highscore.h"
#include "input.h"
#include "joy.h"
#include "mainmenu.h"
#include "menu.h"
#include "options.h"
#include "platform.h"
#include "sound.h"
#include "sprite.h"
#include "system.h"
#include "task.h"

static Uint8 palette[] = {
        0x0F, 0x36, 0x26, 0x16,
        0x0F, 0x11, 0x21, 0x31,
        0x0F, 0x12, 0x22, 0x32,
        0x0F, 0x13, 0x23, 0x33,
        0x0F, 0x36, 0x26, 0x16,
        0x0F, 0x11, 0x21, 0x31,
        0x0F, 0x12, 0x22, 0x32,
        0x0F, 0x13, 0x23, 0x33,
};

struct {
    char *name;
    Uint32 button;
} buttonNames[] = {
    {"Up", JOY_UP},
    {"Down", JOY_DOWN},
    {"Left", JOY_LEFT},
    {"Right", JOY_RIGHT},
    {"B", JOY_B},
    {"A", JOY_A},
    {"Select", JOY_SELECT},
    {"Start", JOY_START},
};

#define KEYBOARD_CONTROLS 0
#define GAMEPAD_CONTROLS 1
static int mapType;
static int last = INPUT_INVALID;
static void Options_InputCallback(int button) {
    if ((mapType == KEYBOARD_CONTROLS) && (button >= INPUT_KEY_MIN) && (button <= INPUT_KEY_MAX)) {
        last = button;
    }
    else if ((mapType == GAMEPAD_CONTROLS) && (button >= INPUT_GAMEPAD_MIN) && (button <= INPUT_GAMEPAD_MAX)) {
        last = button;
    }
}

void Options_Map(void) {
    BG_Clear();
    Sprite_ClearList();

    // show a message if the user tried to map a gamepad and there isn't one connected
    if ((mapType == GAMEPAD_CONTROLS) && !Platform_GamepadConnected()) {
        BG_Print(5, 8, 0, "Gamepad not connected.");
        for (int i = 0; i < 60; i++) {
            BG_Display();
            Task_Yield();
        }
        return;
    }

    Input_SetOnPressFunc(Options_InputCallback);
    for (int i = 0; i < ARRAY_LEN(buttonNames); i++) {
        while (1) {
            BG_Print(2, (i * 2) + 2, 0, "Press key for %s", buttonNames[i].name);
            if (last != INPUT_INVALID) {
                BG_Print(24, (i * 2) + 2, 0, "%s", Input_ButtonName(last));
                if (mapType == KEYBOARD_CONTROLS) {
                    Joy_MapKey(last, buttonNames[i].button);
                }
                else {
                    Joy_MapGamepad(last, buttonNames[i].button);
                }
                last = INPUT_INVALID;
                break;
            }
            BG_Display();
            Task_Yield();
        }
        // wait a few frames to debounce
        for (int j = 0; j < 10; j++) {
            BG_Display();
            Task_Yield();
        }
    }
    Input_SetOnPressFunc(NULL);
    // save key mappings to disk
    Joy_SaveMappings();
}

void controlsDraw(void) {
    if (mapType == KEYBOARD_CONTROLS) {
        BG_Print(8, 2, 0, "Keyboard Controls");
    }
    else {
        BG_Print(8, 2, 0, "Gamepad Controls");
    }
    for (int i = 0; i < ARRAY_LEN(buttonNames); i++) {
        if (mapType == KEYBOARD_CONTROLS) {
            BG_Print(6, (i * 2) + 6, 0, "%-10s %s", buttonNames[i].name,
                     Joy_StrKey(buttonNames[i].button));
        }
        else {
            BG_Print(6, (i * 2) + 6, 0, "%-10s %s", buttonNames[i].name,
                     Joy_StrGamepad(buttonNames[i].button));
        }
    }
}

static MenuItem controlsItems[] = {
    MENU_LINK("Map", Options_Map),
    MENU_TASK("Back", Options_Run),
};

static char *boolOptions[] = {"OFF", "ON"};

static int fullscreenCB(int num) {
    num &= 1;
    return Platform_SetFullscreen(num);
}

static int overscanCB(int num) {
    num &= 1;
    return Platform_SetOverscan(num);
}

static int ntscCB(int num) {
    num &= 1;
    return Platform_SetNTSC(num);
}

static void keyboardTask(void) {
    mapType = KEYBOARD_CONTROLS;
    Menu_Run(12, 24, 2, controlsItems, ARRAY_LEN(controlsItems), controlsDraw);
}

static void gamepadTask(void) {
    mapType = GAMEPAD_CONTROLS;
    Menu_Run(12, 24, 2, controlsItems, ARRAY_LEN(controlsItems), controlsDraw);
}

static char *gameTypeOptions[] = {"Original", "Plus", "Arcade"};

static int gameTypeInit(void) { return gameType; }

static int gameTypeCB(int num) {
    if (num < 0) { num = ARRAY_LEN(gameTypeOptions) - 1; }
    if (num >= ARRAY_LEN(gameTypeOptions)) { num = 0; }
    gameType = num;
    if (gameType == GAME_TYPE_ARCADE) {
        Platform_SetPaletteType(PALETTE_TYPE_2C04);
    }
    else {
        Platform_SetPaletteType(PALETTE_TYPE_NES);
    }
    DB_Set("gametype", &gameType, 1);
    DB_Save();
    return num;
}

static char *arcadeDiffOptions[] = {"Easy", "Normal", "Hard"};

static int arcadeDiffInit(void) { return arcadeDifficulty; }

static int arcadeDiffCB(int num) {
    if (num < 0) { num = ARRAY_LEN(arcadeDiffOptions) - 1; }
    if (num >= ARRAY_LEN(arcadeDiffOptions)) { num = 0; }
    arcadeDifficulty = num;
    DB_Set("arcadediff", &arcadeDifficulty, 1);
    DB_Save();
    return num;
}

static char *arcadeColorOptions[] = {"Raw RGB", "Corrected"};

static int arcadeColorCB(int num) {
    num &= 1;
    return Platform_SetArcadeColor(num);
}

static void doHighScoreReset(void) {
    HighScore_ResetScores();
    Task_Switch(Options_Run);
}

static MenuItem highScoreItems[] = {
    MENU_TASK("Reset", doHighScoreReset),
    MENU_TASK("Back", Options_Run),
};

static void highScoreResetDraw(void) {
    BG_Print(7, 2, 0, "Reset High Scores");
    BG_Print(4, 6, 0, "Are you sure you want to\n\nreset the arcade mode\n\nhigh score table?");
}

static void highScoreResetTask(void) {
    Menu_Run(12, 14, 2, highScoreItems, ARRAY_LEN(highScoreItems), highScoreResetDraw);
}

static MenuItem optionsItems[] = {
    MENU_LIST("Fullscreen", boolOptions, Platform_GetFullscreen, fullscreenCB),
    MENU_NUM("Window scale", Platform_GetVideoScale, Platform_SetVideoScale, 1),
    MENU_LIST("Overscan", boolOptions, Platform_GetOverscan, overscanCB),
    MENU_LIST("NTSC filter", boolOptions, Platform_GetNTSC, ntscCB),
    MENU_NUM("Volume", Sound_GetVolume, Sound_SetVolume, 5),
    MENU_TASK("Keyboard controls", keyboardTask),
    MENU_TASK("Gamepad controls", gamepadTask),
    MENU_LIST("Game type", gameTypeOptions, gameTypeInit, gameTypeCB),
    MENU_LIST("Arcade difficulty", arcadeDiffOptions, arcadeDiffInit, arcadeDiffCB),
    MENU_LIST("Arcade color", arcadeColorOptions, Platform_GetArcadeColor, arcadeColorCB),
    MENU_TASK("Reset high scores", highScoreResetTask),
    MENU_TASK("Back", MainMenu_Run),
};

void Options_Draw(void) {
    BG_Print(12, 2, 0, "Options");
}

void Options_Run(void) {
    BG_SetAllPalettes(palette);
    Sprite_SetAllPalettes(palette + 16);
    Menu_Run(3, 5, 2, optionsItems, ARRAY_LEN(optionsItems), Options_Draw);
}
