/* hud.h: HUD display code
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

void HUD_DisplayOriginal(Sint16 health, Sint16 magic);
void HUD_DisplayPlus(Sint16 health, Sint16 magic);
void HUD_DisplayArcade(Sint16 health, Sint16 magic, Uint32 score);

/**
 * @brief Displays the current weapon
 * @param x display x coordinate
 * @param y display y coordinate
 */
void HUD_Weapon(Sint16 x, Sint16 y);
