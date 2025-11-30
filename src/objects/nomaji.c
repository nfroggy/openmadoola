/* nomaji.c: Nomaji object code
 * Copyright (c) 2023 Nathan Misner
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
#include "game.h"
#include "lucia.h"
#include "map.h"
#include "nomaji.h"
#include "object.h"
#include "rng.h"
#include "sound.h"
#include "sprite.h"

void Nomaji_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->hp = 20;
    o->type += 0x20;
}

void Nomaji_Obj(Object *o) {
    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.tile = 0x82;

    if (gameType == GAME_TYPE_ARCADE) {
        Fireball_Spawn(0x3f, o);
    }
    if (o->stunnedTimer == 0) {
        if (Object_TouchingGround(o)) {
            if (((rngVal + gameFrames) & 0xFF) < 0xF0) {
                goto doneUpdatePos;
            }
            
            // jump towards lucia with a random x speed
            Sound_Play(SFX_NOMAJI);
            o->xSpeed = RNG_Get() & 0x3f;
            if (luciaXPos.f.h < o->x.f.h) {
                o->xSpeed = -o->xSpeed;
            }
            o->ySpeed = -0x80;
        }

        spr.tile = 0xa2;
        Object_CheckForWall(o);
        Object_SetDirection(o);
        Object_ApplyGravity(o);
        if (Object_UpdateYPos(o)) {
            o->ySpeed = 0x40;
        }
    }
doneUpdatePos:
    spr.palette = 3;
    if (Object_GetMetatile(o) >= MAP_SOLID) {
        if (o->stunnedTimer) {
            o->stunnedTimer--;
        }

        if (Sprite_SetDraw(&spr, o, 0, 0)) {
            Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 3);
            return;
        }
    }

    o->type = OBJ_NONE;
}