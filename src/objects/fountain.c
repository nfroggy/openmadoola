/* fountain.c: Fountain object code
 * Copyright (c) 2023-2026 Nathan Misner
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
#include "item.h"
#include "object.h"
#include "sprite.h"

void Fountain_Obj(Object *o) {
    o->x.f.l = 0x80;
    o->y.f.l = 0x80;
    // animate water by mirroring the middle and bottom sprites
    o->direction = (gameFrames << 4) & DIR_LEFT;
    o->stunnedTimer = 0;
    
    Sprite spr = { 0 };
    // draw middle of the water sprite
    spr.tile = 0x59C;
    spr.palette = 2;
    spr.size = SPRITE_16X16;
    if (Sprite_SetDraw(&spr, o, 0, 0)) {
        // draw bottom of the water sprite
        spr.y += 16;
        spr.tile = 0x5AE;
        Sprite_Draw(&spr, NULL);

        // draw top of the water sprite
        spr.y -= 32;
        if (o->direction == DIR_LEFT) {
            spr.tile = 0x560;
        }
        else {
            spr.tile = 0x540;
        }
        spr.mirror = 0;
        Sprite_Draw(&spr, NULL);

        // give lucia the equivalent of a 500 hp powerup every 16 frames
        if ((gameFrames & 0xf) == 0) {
            spr.y += 16;
            if (Collision_WithLucia(o, &spr, COLLISION_SIZE_16X32, ITEM_FLAG + ITEM_APPLE) == 2) {
                fountainUsed = 1;
            }
        }
    }
}