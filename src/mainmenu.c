/* mainmenu.c: Main menu screen
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

#include "bg.h"
#include "constants.h"
#include "menu.h"
#include "options.h"
#include "platform.h"
#include "save.h"
#include "soundtest.h"
#include "sprite.h"
#include "title.h"

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

static MenuItem items[] = {
    MENU_TASK("Start Game", Save_Screen),
    MENU_TASK("Options", Options_Run),
    MENU_TASK("Sound Test", SoundTest_Run),
    MENU_TASK("Title Screen", Title_Run),
    MENU_LINK("Quit", Platform_Quit),
};

void MainMenu_Run(void) {
    BG_SetAllPalettes(palette);
    Sprite_SetAllPalettes(palette + 16);
    Menu_Run(10, 7, 3, items, ARRAY_LEN(items), NULL);
}