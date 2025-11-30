/* dopipu.c: Dopipu object code
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
#include "dopipu.h"
#include "map.h"
#include "object.h"
#include "sprite.h"

void Dopipu_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->hp = 7;
    o->type += 0x20;
    o->timer = 0;
    o->stunnedTimer = 0;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x10 : -0x10;
    o->ySpeed = 0;
}

void Dopipu_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    else {
        // make dopipu jump
        if (o->timer == 0) {
            if (Object_TouchingGround(o)) {
                if (!Object_UpdateXPos(o)) {
                    goto doneUpdatePos;
                }
            }

            o->ySpeed = -0x80;
            o->timer++;
        }

        if (o->ySpeed < 0) {
            Object_ApplyGravity(o);
            // make dopipu start falling if it hits the ceiling
            if (Object_UpdateYPos(o)) {
                o->ySpeed = 0;
            }
        }
        else {
            Object_ApplyGravity(o);
            Object_CheckForWall(o);
            if (Object_UpdateYPos(o)) {
                o->y.f.l &= 0x80;
                o->timer--;
            }
        }
    }
doneUpdatePos:;
    Uint16 metatile = Object_GetMetatile(o);
    if (metatile < MAP_SOLID) {
        goto despawn;
    }
    
    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 1;

    if (o->timer != 0) {
        spr.tile = 0xA8;
    }
    else if (o->x.f.l & 0x80) {
        spr.tile = 0xCA;
    }
    else if (o->x.f.h & 1) {
        spr.tile = 0xAA;
    }
    else {
        spr.tile = 0xA8;
    }
    
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        goto despawn;
    }

    Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 10);
    return;

despawn:
    o->type = OBJ_NONE;
}