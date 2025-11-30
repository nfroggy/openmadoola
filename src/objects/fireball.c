/* fireball.c: Fireball object code
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

#include "collision.h"
#include "constants.h"
#include "fireball.h"
#include "game.h"
#include "object.h"
#include "rng.h"
#include "sound.h"

void Fireball_Obj(Object *o) {
    o->timer--;
    if (!o->timer) {
        o->type = OBJ_NONE;
        return;
    }

    if (o->timer >= 115) {
        Object_MoveTowardsLucia(o);
    }

    Object_CalcXYPos(o);
    Object_ApplyGravity(o);
    Sprite spr = { 0 };
    spr.size = SPRITE_8X16;
    spr.palette = 0;
    spr.tile = 0x48;
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        return;
    }

    Collision_WithLucia(o, &spr, COLLISION_SIZE_16X16, 20);
}

void Fireball_Spawn(Uint8 mask, Object *parent) {
    if (!(rngVal & mask)) {
        RNG_Get();
        Object *o = Object_FindNext(9, 17);
        if (o) {
            o->x = parent->x;
            o->y = parent->y;
            o->timer = 120;
            o->stunnedTimer = 0;
            o->ySpeed = -0x80;
            o->xSpeed = RNG_Get() & 0x3f;
            o->type = OBJ_FIREBALL;
            Object_FaceLucia(o);
            Sound_Play(SFX_FIREBALL);
        }
    }
}