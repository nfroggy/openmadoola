/* system.c: Engine management code
 * Copyright (c) 2023, 2024, 2025 Nathan Misner
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

#include <assert.h>
#include "db.h"
#include "game.h"
#include "highscore.h"
#include "joy.h"
#include "palette.h"
#include "platform.h"
#include "rng.h"
#include "rom.h"
#include "save.h"
#include "sound.h"
#include "system.h"
#include "task.h"

int System_Init(void) {
    // load assets
    if (!Rom_Load())                                { return 0; }
    if (!Rom_LoadChr("font.bin", 4096)) { return 0; }
    DB_Init();
    Game_LoadSettings();

    // initialize platform code
    if (!Platform_Init()) { return 0; }

    // initialize engine components
    if (!Graphics_Init()) { return 0; }
    Sound_Init();
    Save_Init();
    HighScore_Init();
    Joy_Init();
    RNG_Seed();
    return 1;
}

void System_GameLoop(void) {
    while (1) {
        Platform_StartFrame();
        Graphics_StartFrame();
        Joy_Update();
        Task_Run();
        Sound_Run();
        Platform_EndFrame();
    }
}
