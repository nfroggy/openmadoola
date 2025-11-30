/* spajyan.c: Spajyan object code
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
#include "game.h"
#include "map.h"
#include "object.h"
#include "spajyan.h"
#include "sprite.h"

void Spajyan_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->hp = 3;
    o->type += 0x20;
    o->timer = 0;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x8 : -0x8;
    o->ySpeed = -0x80;
}

void Spajyan_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    else {
        // walk around on the ground
        if (o->timer) {
            Object_CheckForDrop(o);
            Object_CheckForWall(o);
            o->timer++;
        }
        // do jump
        else {
            Object_ApplyGravity(o);
            Object_CheckForWall(o);
            Object_CheckForWall(o);
            Object_CheckForWall(o);
            Object_CheckForWall(o);
            // handle landing on ground
            if (Object_UpdateYPos(o)) {
                if (Object_TouchingGround(o)) {
                    o->y.f.l &= 0x80;
                    o->timer += 0x80;
                    o->ySpeed = -0x80;
                }
                else {
                    o->ySpeed = 0;
                }
            }
        }
    }

    if (Object_GetMetatile(o) < MAP_SOLID) {
        o->type = OBJ_NONE;
        return;
    }

    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 3;
    // legs extended tile
    if ((!o->timer) || (!(o->x.f.l & 0x40))) {
        spr.tile = 0x1aa;
    }
    // legs tucked tile
    else {
        spr.tile = 0x18a;
    }
    
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }

    Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 8);
}