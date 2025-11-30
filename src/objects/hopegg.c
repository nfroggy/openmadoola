/* hopegg.c: Hopegg object code
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
#include "hopegg.h"
#include "map.h"
#include "object.h"
#include "sprite.h"

void Hopegg_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->hp = 30;
    o->type += 0x20;
    o->timer = 0;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x20 : -0x20;
}

void Hopegg_Obj(Object *o) {
    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;

    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    else if (o->timer < 0xA0) {
        o->timer++;
    }
    else {
        if (o->timer == 0xA0) {
            Object_MoveTowardsLucia(o);
        }

        // bounce twice
        if ((o->timer == 0xA0) || (o->timer == 0xD0)) {
            o->timer++;
            o->ySpeed = -0x80;
        }

        Object_ApplyGravity(o);
        Object_CheckForWall(o);
        if (Object_UpdateYPos(o)) {
            if (Object_TouchingGround(o)) {
                o->y.f.l &= 0x80;
                o->timer += 0x2F;
            }

            o->ySpeed = 0;
        }
    }

    // if inside ground, erase object
    if (Object_GetMetatile(o) < MAP_SOLID) {
        o->type = OBJ_NONE;
        return;
    }

    o->y.f.h--;
    spr.palette = 1;
    // draw top of hopegg
    spr.tile = 0x1e8;
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }

    spr.y += 16;

    if (o->timer < 0xA0) {
        // looking left and right animation
        if ((o->timer & 0x20) == 0) {
            o->direction = DIR_LEFT;
        }
        else {
            o->direction = DIR_RIGHT;
        }
        spr.tile = 0x1ea;
        Sprite_Draw(&spr, o);
    }
    else {
        // don't show eyes when hopping
        spr.size = SPRITE_8X16;
        spr.x -= 4;
        o->direction = DIR_RIGHT;
        spr.tile = 0x1fa;
        Sprite_DrawDir(&spr, o);
        spr.x += 8;
        o->direction = DIR_LEFT;
        Sprite_DrawDir(&spr, o);
        spr.x -= 4;
    }
    
    spr.y -= 16;
    o->y.f.h++;
    Collision_Handle(o, &spr, COLLISION_SIZE_16X32, 25);
}