/* camera.h: Camera handling
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

#pragma once
#include "constants.h"
#include "object.h"

extern Fixed16 cameraX;
extern Fixed16 cameraY;

#define SCROLL_MODE_FREE (0)
#define SCROLL_MODE_X (1)
#define SCROLL_MODE_LOCKED (2)
extern Uint8 scrollMode;

/**
 * @brief Centers the camera around the given object.
 * @param o The object to focus on
 */
void Camera_SetXY(Object *o);

/**
 * @brief Updates the camera based on Lucia's position
 * @param o Pointer to Lucia's object
 */
void Camera_LuciaScroll(Object *o);
