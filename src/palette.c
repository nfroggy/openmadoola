/* palette.c: Color palette
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

#include "constants.h"
#include "file.h"
#include "game.h"
#include "graphics.h"
#include "palette.h"
#include "platform.h"

// which NES colors to use
Uint8 colorPalette[PALETTE_SIZE * 12];
// palette to use when flashTimer is nonzero
static Uint8 flashPalette[ARRAY_LEN(colorPalette)];
Uint8 flashTimer = 0;

Uint8 *Palette_Run(void) {
    if (flashTimer) {
        flashTimer--;
        for (int i = 0; i < ARRAY_LEN(colorPalette); i++) {
            flashPalette[i] = (((gameFrames << 2) & 0x30) + colorPalette[i]) & 0x3f;
        }
        return flashPalette;
    }
    return colorPalette;
}
