/* sword.c: Sword object code
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
#include "lucia.h"
#include "object.h"
#include "sprite.h"
#include "sword.h"
#include "weapon.h"

static Sint8 swordXOffsets[] = {
    -0x03, 0x10, 0x03, -0x10,
};

static Sint8 swordYOffsets[] = {
    -0x10, 0x06, -0x10, 0x06,
};

static Sint8 swordMirrors[] = {
    H_MIRROR, H_MIRROR, 0, 0,
};

static Sint8 swordTiles[] = {
    0x40, 0x42, 0x40, 0x42,
};

void Sword_Obj(Object *o) {
    if (attackTimer) {
        Sprite spr = { 0 };
        spr.palette = 1;
        
        // handle animation
        Uint8 swordIndex = 0;
        if (attackTimer < 6) {
            swordIndex++;
        }

        // match lucia's mirroring
        if (objects[0].direction) {
            swordIndex += 2;
        }

        spr.x = luciaSpriteX + swordXOffsets[swordIndex];
        spr.y = luciaSpriteY - 16 + swordYOffsets[swordIndex];
        spr.tile = swordTiles[swordIndex];
        spr.mirror = swordMirrors[swordIndex];
        spr.size = SPRITE_16X16;
        Sprite_Draw(&spr, NULL);
        // if the sword collided with an enemy, don't return and delete the object
        // and collision coordinates
        if (!Weapon_SetCollisionCoords(spr.x, spr.y)) {
            return;
        }
        attackTimer = 0;
    }

    o->type = OBJ_NONE;
    Weapon_EraseCollisionCoords();
}