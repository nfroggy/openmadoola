/* nishiga.c: Nishiga object code
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
#include "nishiga.h"
#include "object.h"
#include "sprite.h"

void Nishiga_InitObj(Object *o) {
    o->hp = 8;
    o->type += 0x20;
    o->timer = 0;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x20 : -0x20;
    o->ySpeed = 0;
}

void Nishiga_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    else {
        // move down
        if (!(o->timer & 0x20)) {
            o->ySpeed = -0x10;
        }
        // move up
        else if (!(o->timer & 0x40)) {
            o->ySpeed = 0x10;
        }
        //move towards lucia
        else {
            if (!(o->timer & 0x1f)) {
                Object_MoveTowardsLucia(o);
            }
            Object_CheckForWall(o);
            o->timer++;
            o->ySpeed = 0x20;
        }
        Object_UpdateYPos(o);
        o->timer++;
    }

    // erase object if in solid ground
    if (Object_GetMetatile(o) < MAP_SOLID) {
        o->type = OBJ_NONE;
        return;
    }

    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 2;

    // flapping animation
    if (o->timer & 2) {
        spr.tile = 0x188;
    }
    else {
        spr.tile = 0x1a8;
    }
    
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }
    Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 8);
}