/* save.h: Save file handler
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

/**
 * @brief Loads any save files from disk. Should be run at startup.
 */
void Save_Init(void);

/**
 * @brief Saves game state to disk.
 */
void Save_SaveFile(void);

#define SAVE_SCREEN_BACK 0
#define SAVE_SCREEN_NEWGAME 1
#define SAVE_SCREEN_LOADGAME 2

/**
 * @brief Displays the save file screen.
 */
void Save_Screen(void);