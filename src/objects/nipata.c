/* nipata.c: Nipata object code
 * Copyright (c) 2023, 2024 Nathan Misner
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
#include "constants.h"
#include "game.h"
#include "map.h"
#include "nipata.h"
#include "object.h"
#include "rng.h"
#include "sprite.h"

void Nipata_InitObj(Object *o) {
    // It looks like the original code had something similar to this but they
    // commented it out, we need it because unlike on the NES, doing an out of
    // bounds access on the collision array will make the game crash.
    if (Object_CollisionOutOfBounds(o)) {
        o->type = OBJ_NONE;
        return;
    }
    
    o->hp = 5;
    o->type += 0x20;
    o->timer = 0x5e;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x20 : -0x20;
    o->ySpeed = 0;
}

void Nipata_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    // flying up
    else if (!(o->timer & 0xc0)) {
        Object_CheckForWall(o);
        int hitCeiling;
        // NOTE: This fixes a bug where if nipata spawned between two walls, it would
        // slowly rise up and then snap back down over and over.
        if (gameType != GAME_TYPE_ORIGINAL) {
            Object_CalcYPos(o);
            hitCeiling = Map_SolidTileAbove(o->collision);
        }
        else {
            hitCeiling = Object_UpdateYPos(o);
        }
        if (hitCeiling) {
            o->ySpeed = 0;
            o->y.f.l &= 0x80;
            o->timer |= 0x8f;
        }
    }
    // swooping down off the ceiling
    else if (o->timer & 0x40) {
        Object_CheckForWall(o);
        Object_ApplyGravity(o);
        // bounce off the floor
        if (Object_UpdateYPos(o)) {
            o->ySpeed = 0xf0;
        }
        else {
            if ((o->timer & 0x30) != 0x30) {
                o->timer--;
            }
            o->timer--;
            if (!(o->timer & 0xf)) {
                o->ySpeed = 0xf8;
                do {
                    o->timer &= 0x3f;
                    o->timer += 0x10;
                } while (o->timer & 0x40);
            }
        }
    }
    // hanging out on the ceiling
    else if (!(RNG_Get() & 1)) {
        o->timer--;
        if (!(o->timer & 0xf)) {
            o->timer |= 0x4e;
            o->timer &= 0x7f;
            o->xSpeed = -o->xSpeed;
            o->direction ^= 0x80;
            o->ySpeed = 0;
        }
    }

    if (Object_GetMetatile(o) < MAP_SOLID) {
        o->type = OBJ_NONE;
        return;
    }
    Sprite spr = { 0 };
    spr.palette = 0;
    if ((o->timer < 0x80) && (!(o->x.f.h & 1))) {
        spr.tile = 0x80;
    }
    else {
        spr.tile = 0x90;
    }
    spr.size = SPRITE_8X16;
    o->direction = 0x80;
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }
    o->direction = DIR_RIGHT;
    spr.x += 8;
    Sprite_DrawDir(&spr, o);
    o->direction = (o->xSpeed >= 0) ? DIR_RIGHT : DIR_LEFT;
    spr.x -= 8;
    Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 8);
}
