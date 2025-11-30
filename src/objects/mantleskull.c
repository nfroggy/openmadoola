/* mantleskull.c: Mantle Skull object code
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
#include "fireball.h"
#include "mantleskull.h"
#include "map.h"
#include "object.h"
#include "sprite.h"

void MantleSkull_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->hp = 80;
    o->type += 0x20;
    o->timer = 0;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x18 : -0x18;
    o->ySpeed = 0;
}

void MantleSkull_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    else {
        Fireball_Spawn(0x3F, o);
        if (o->timer == 0) {
            if (Object_TouchingGround(o)) {
                if (!Object_UpdateXPos(o)) {
                    goto doneMovement;
                }
            }
            o->ySpeed = -0x80;
            o->timer++;
        }

        // object jumping
        if (o->ySpeed < 0) {
            Object_ApplyGravity(o);
            if (!Object_UpdateYPos(o)) {
                goto doneMovement;
            }
            o->ySpeed = 0;
        }
        else {
            Object_ApplyGravity(o);
            Object_CheckForWall(o);
            if (!Object_UpdateYPos(o)) {
                goto doneMovement;
            }
            o->y.f.l &= 0x80;
            o->timer--;
        }
    }

doneMovement:
    if (Object_GetMetatile(o) < MAP_SOLID) {
        o->type = OBJ_NONE;
        return;
    }

    Sprite spr = { 0 };
    spr.palette = 0;
    if (o->x.f.l & 0x80) {
        spr.tile = 0xA6;
    }
    else {
        spr.tile = 0x86;
    }
    
    if (!Sprite_SetDraw16x32(&spr, o, spr.tile - 2, 0, 0)) {
        return;
    }

    Collision_Handle(o, &spr, COLLISION_SIZE_16X32, 40);
}