/* pausemenu.h: Game pause menu
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

#pragma once

/**
 * @brief Initializes pause menu. Must be run when the player hits start to pause the game.
 */
void PauseMenu_Init(void);

#define PAUSE_EXIT_NONE 0
#define PAUSE_EXIT_RESUME 1
#define PAUSE_EXIT_QUIT 2
/**
 * @brief Runs the pause menu. Must be run every frame the game is paused.
 * @returns PAUSE_EXIT_NONE if the player didn't do anything, PAUSE_EXIT_RESUME if the player
 * chose to resume the game, PAUSE_EXIT_QUIT if the player chose to quit.
 */
int PauseMenu_Run(void);
