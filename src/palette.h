/* palette.h: Color palette
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
#include "graphics.h"

// first 4: background next 8: sprites
extern Uint8 colorPalette[];
extern Uint8 flashTimer;

/**
 * @brief Sets up the color palette. Should be run at the start of each frame.
 * @returns the palette that should be used to display this frame (flash palette
 * if flashTimer is nonzero, otherwise the standard palette)
*/
Uint8 *Palette_Run(void);
