/* fire.c: Fire object code
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
#include "fire.h"
#include "map.h"
#include "object.h"
#include "sprite.h"

void Fire_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->hp = 35;
    o->type += 0x20;
    o->timer = 0;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x8 : -0x8;
    o->ySpeed = 0;
}

static Sint8 fireOffsetTbl[] = {
    0, -3, -5, -3, 0, 3, 5, 3,
};

void Fire_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    else {
        if (o->ySpeed == 0) {
            if (Object_UpdateXPos(o)) {
                o->timer += 0x40;
                if (o->timer & 0xc0) {
                    o->direction ^= 0x80;
                    o->xSpeed = -o->xSpeed;
                }
                else {
                    o->ySpeed = -0x80;
                }
            }
            else {
                o->timer &= 0x3f;
                if (!Object_TouchingGround(o)) {
                    Object_ApplyGravity(o);
                    if (Object_UpdateYPos(o)) {
                        o->ySpeed = 0;
                        o->y.f.l &= 0x80;
                    }
                }
            }
        }
        else {
            Object_CheckForWall(o);
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
    spr.palette = 0;
    spr.size = SPRITE_16X16;
    spr.tile = 0xc8;
    o->timer++;
    if (!(o->timer & 0x3f)) {
        o->timer -= 0x40;
    }
    if (!Sprite_SetDraw(&spr, o, 0, fireOffsetTbl[(o->timer & 0x1c) >> 2])) {
        o->type = OBJ_NONE;
        return;
    }
    spr.tile = 0xa0;
    spr.y -= 0x10;
    Sprite_Draw(&spr, o);
    spr.y += 0x10;
    Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 25);
}
