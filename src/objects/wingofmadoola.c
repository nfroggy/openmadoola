/* wingofmadoola.c: Object code for the titular Wing of Madoola
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
#include "game.h"
#include "item.h"
#include "lucia.h"
#include "object.h"
#include "sprite.h"
#include "wingofmadoola.h"

void WingOfMadoola_Obj(Object *o) {
    Sprite spr = { 0 };
    spr.size = SPRITE_8X16;

    o->direction = DIR_RIGHT;
    o->stunnedTimer = 0;
    o->x.v = 0x3800;
    o->y.v = 0x3d80;
    if (!Sprite_SetPos(&spr, o, 0, 0)) {
        return;
    }
    spr.tile = 0x6ce;
    spr.palette = 3;
    // guessing this was done here rather than in the object position constant
    // to influence when the object gets hidden/shown
    spr.x -= 8;
    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < 3; x++) {
            Sprite_Draw(&spr, o);
            spr.x += 8;
            spr.tile += 0x10;
        }
        spr.tile -= 0x32;
        spr.y -= 0x10;
        spr.x -= 0x18;
    }

    spr.x += 8;
    spr.y += 0x10;
    // give lucia 500 MP
    if (Collision_WithLucia(o, &spr, COLLISION_SIZE_16X32, ITEM_FLAG + ITEM_SCROLL) == 2) {
        hasWing = 0xff;
        Object_DeleteAllAfterLucia();
        o->type = OBJ_DALTOS_INIT;
        Game_PlayRoomSong();
    }
}