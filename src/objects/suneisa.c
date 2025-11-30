/* suneisa.c: Suneisa object code
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
#include "map.h"
#include "object.h"
#include "sprite.h"
#include "suneisa.h"

static Sint8 offsets[] = {
//   x     y
     0x00, -0x10,
     0x10,  0x10,
    -0x10, -0x10,
};

static Uint16 frame1[] = {
    0x182, 0x180, 0x1a2, 0x00,
};

static Uint16 frame2[] = {
    0x1ca, 0x1c8, 0x1a0, 0x00,
};

void Suneisa_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->hp = 128;
    o->type += 0x20;
    o->timer = 0;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x20 : -0x20;
    o->ySpeed = 0;
}

void Suneisa_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    else {
        if (!o->ySpeed) {
            if (!Object_TouchingGround(o)) {
                if (!o->timer) {
                    o->timer++;
                }
                else {
                    o->timer--;
                    goto handleFalling;
                }
            }
            Object_CheckForDrop(o);
            Object_CheckForWall(o);
        }
        else {
        handleFalling:
            Object_ApplyGravity(o);
            if (Object_UpdateYPos(o)) {
                o->ySpeed = 0;
                o->y.f.l &= 0x80;
            }
        }
    }

    if (Object_GetMetatile(o) < MAP_SOLID) {
        o->type = OBJ_NONE;
        return;
    }

    Sprite spr = { 0 };
    spr.palette = 3;
    Uint16 *frame;
    if (!(o->x.f.l & 0x80)) {
        frame = frame1;
    }
    else {
        frame = frame2;
    }
    
    if (!Sprite_SetDrawLarge(&spr, o, frame, offsets, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }

    Collision_Handle(o, &spr, COLLISION_SIZE_16X32, 40);
}