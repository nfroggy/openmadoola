/* peraskull.c: Peraskull object code
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
#include "lucia.h"
#include "map.h"
#include "object.h"
#include "peraskull.h"
#include "sound.h"
#include "sprite.h"

void Peraskull_InitObj(Object *o) {
    // It looks like the original code had something similar to this but they
    // commented it out, we need it because unlike on the NES, doing an out of
    // bounds access on the collision array will make the game crash.
    if (Object_CollisionOutOfBounds(o)) {
        o->type = OBJ_NONE;
        return;
    }

    o->hp = 25;
    o->type += 0x20;
    o->timer = 0x4c;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x10 : -0x10;
    o->ySpeed = 0;
}

void Peraskull_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    else {
        if (o->timer) {
            // falling
            if (!(o->timer & 0x80)) {
                Object_CheckForWall(o);
                Object_ApplyGravity(o);
                if (Object_UpdateYPos(o)) {
                    o->ySpeed = -0x10;
                }
                else {
                    o->timer--;
                    if (o->timer & 0x3f) {
                        goto doneSetPos;
                    }
                    o->ySpeed = -0x10;
                }
                o->timer = 0;
            }
            // rising
            else {
                o->timer--;
                if (!(o->timer & 0x3f)) {
                    o->timer = 0x4c;
                    o->ySpeed = 0;
                }
            }
        }
        // on ceiling
        else {
            Object_CheckForWall(o);
            if (Object_UpdateYPos(o)) {
                o->ySpeed = 0;
                o->y.f.l &= 0x80;
                o->timer = 0x9f;
            }
        }
    }
doneSetPos:

    if (Object_GetMetatile(o) < MAP_SOLID) {
        o->type = OBJ_NONE;
        return;
    }

    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 1;

    if (!(o->timer & 0x80)) {
        if (!(o->x.f.l & 0x70)) {
            Sound_Play(SFX_PERASKULL);
        }
        if (!(o->x.f.l & 0x40)) {
            spr.tile = 0x88;
        }
        else {
            spr.tile = 0x8a;
        }
    }
    else {
        spr.tile = 0xea;
    }

    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }

    Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 20);
}