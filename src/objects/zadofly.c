/* zadofly.c: Zadofly object code
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
#include "sprite.h"
#include "zadofly.h"

void Zadofly_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->y.f.h -= 2;
    Object_InitCollision(o);
    o->hp = 128;
    o->type += 0x20;
    o->timer = 1;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x26 : -0x26;
    o->ySpeed = 0;
}

void Zadofly_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    else {
        if (o->timer == 0) {
            if (!o->ySpeed) {
                if (Object_TouchingGround(o)) {
                    Object_CheckForWall(o);
                    goto doneSetPos;
                }
                else {
                    o->ySpeed = -0x80;
                }
            }

            Object_ApplyGravity(o);
            Object_CheckForWall(o);
            Object_UpdateYPos(o);
            if (o->ySpeed >= 0) {
                o->ySpeed = 0;
                o->timer = 2;
                o->y.f.l &= 0x80;
            }
        }
        else if (o->timer == 2) {
            if (Object_UpdateXPos(o)) {
                o->timer = 0;
            }
        }
        else {
            if (!o->ySpeed) {
                if (ABS(o->x.f.h - luciaXPos.f.h) < 5) {
                    o->ySpeed = 0;
                }
                else {
                    Object_CheckForWall(o);
                    goto doneSetPos;
                }
            }

            Object_ApplyGravity(o);
            Object_CheckForWall(o);
            if (Object_UpdateYPos(o)) {
                o->ySpeed = 0;
                o->timer = 0;
                o->y.f.l &= 0x80;
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
    if (!o->timer) {
        if (!(o->x.f.l & 0x80)) {
            spr.tile = 0x8e;
        }
        else {
            spr.tile = 0xae;
        }
    }
    else {
        spr.tile = 0xae;
    }
    
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }

    if (!o->timer) {
        spr.tile -= 2;
    }
    else {
        if (!(o->x.f.l & 0x40)) {
            spr.tile = 0x8c;
        }
        else {
            spr.tile = 0xac;
        }
    }
    spr.y -= 0x10;
    Sprite_Draw(&spr, o);

    spr.size = SPRITE_8X16;
    if (spr.tile == 0x8c) {
        spr.tile = 0xe8;
    }
    else {
        spr.tile = 0xf8;
    }
    if (o->direction == DIR_RIGHT) {
        spr.x -= 0xc;
        Sprite_Draw(&spr, o);
        spr.x += 0xc;
    }
    else {
        spr.x += 0xc;
        Sprite_Draw(&spr, o);
        spr.x -= 0xc;
    }

    Collision_Handle(o, &spr, COLLISION_SIZE_16X32, 40);
}