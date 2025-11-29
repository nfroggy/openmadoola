/* magicbomb.c: Magic bomb object code
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

#include "constants.h"
#include "game.h"
#include "joy.h"
#include "magicbomb.h"
#include "object.h"
#include "sound.h"
#include "sprite.h"
#include "weapon.h"

static Sint8 bombSpeedTbl[] = {
    -0x70, -0x50, -0x30, -0x10, 0x10, 0x30, 0x50,
};

void MagicBomb_Obj(Object *o) {
    Sprite spr = { 0 };
    spr.tile = 0x62;
    spr.palette = (gameFrames << 1) & 3;
    spr.size = SPRITE_8X16;

    if (o->timer) { o->timer--; }

    // test for when the magic bomb should split
    if (Object_UpdateXPos(o) == 0) {
        // test for the bomb going offscreen
        if (!Sprite_SetDraw(&spr, o, 0, 0)) {
            Weapon_EraseCollisionCoords();
            o->type = OBJ_NONE;
            return;
        }

        // don't split if the magic bomb hit something, the player is still holding b, or it hasn't been at least 3 frames
        if (!Weapon_SetCollisionCoords(spr.x, spr.y) && ((joy & JOY_B) || o->timer)) {
            return;
        }
    }

    // do the magic bomb split
    Sound_Play(SFX_BOMB_SPLIT);
    for (int i = 0; i < 6; i++) {
        if (objects[i + 2].type) {
            continue;
        }
        else {
            objects[i + 2].x = o->x;
            objects[i + 2].y = o->y;
            objects[i + 2].collision = o->collision;
            objects[i + 2].type = OBJ_MAGIC_BOMB_FIRE;
            objects[i + 2].ySpeed = bombSpeedTbl[i];
            objects[i + 2].xSpeed = 0;
        }
    }

    Weapon_EraseCollisionCoords();
    o->type = OBJ_NONE;
}
