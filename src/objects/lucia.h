/* lucia.h: Lucia object code
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


extern Uint8 bootsLevel;
extern Uint8 attackTimer;
extern Uint8 hasWing;
extern Uint8 usingWing;
extern Uint8 luciaHurtPoints;

extern Sint16 health;
extern Sint16 maxHealth;
extern Sint8 healthTimer;
extern Sint16 magic;
extern Sint16 maxMagic;
extern Sint8 lives;

extern Fixed16 luciaXPos;
extern Fixed16 luciaYPos;
extern Sint16 luciaSpriteX;
extern Sint16 luciaSpriteY;
extern Uint16 luciaMetatile;

void Lucia_NormalObj(Object *o);
void Lucia_LvlEndDoorObj(Object *o);
void Lucia_WarpDoorObj(Object *o);
void Lucia_DyingObj(Object *o);
void Lucia_ClimbObj(Object *o);
void Lucia_AirObj(Object *o);

