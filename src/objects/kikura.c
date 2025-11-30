/* kikura.c: Kikura object code
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
#include "kikura.h"
#include "map.h"
#include "object.h"
#include "sprite.h"

void Kikura_InitObj(Object *o) {
    o->y.f.h += 8;
    o->hp = 8;
    o->type += 0x20;
    o->timer = 0;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x20 : -0x20;
}

void Kikura_Obj(Object *o) {
    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;

    if (o->stunnedTimer) {
        o->stunnedTimer--;
        spr.tile = 0xCC;
    }
    else {
        // every 32 frames, switch between moving up and down/over
        if ((o->timer & 0x20) == 0) {
            // move up, don't update x pos
            o->ySpeed = -0x18;
            spr.tile = 0xCE;
        }
        else {
            // move down and over
            Object_CalcXPos(o);
            o->ySpeed = 0x10;
            spr.tile = 0xCC;
        }
        Object_CalcYPos(o);
        o->timer++;
    }

    spr.palette = 3;
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }

    Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 8);
}