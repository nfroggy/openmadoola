/* bospido.c: Bospido object code
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

#include "bospido.h"
#include "collision.h"
#include "fireball.h"
#include "game.h"
#include "map.h"
#include "object.h"
#include "sprite.h"

static Sint8 bospidoOffsets[] = {
    -0x10,  0x00,
     0x00, -0x10,
     0x10,  0x00,
     0x0c,  0x10,
};

static Uint16 bospido1[] = {
    0x2aa, 0x28a, 0x288, 0x2a8, 0x00,
};

static Uint16 bospido2[] = {
    0x28e, 0x2da, 0x2d8, 0x28c, 0x00,
};

void Bospido_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->hp = 255;
    o->type += 0x20;
    o->timer = 0;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x30 : -0x30;
    o->ySpeed = 0;
}

void Bospido_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    Fireball_Spawn(0xf, o);

    // on ground
    if (!o->ySpeed) {
        o->timer++;
        if (o->timer == 0x40) {
            o->timer = 0;
            o->ySpeed = -0x80;
            Object_MoveTowardsLucia(o);
        }
    }
    // jumping
    else {
        Object_ApplyGravity(o);
        Object_CheckForWall(o);
        if (Object_UpdateYPos(o)) {
            // NOTE: There's a bug from the original game where if Bospido touches the ceiling,
            // it will get stuck. I decided to leave it like that because I think it's kinda fun
            // and if you fix it, Lucia's cornered with Bospido repeatedly bouncing in front of
            // her which is less interesting.
            // An example fix for the bug is to replace "o->ySpeed = 0" with:
            // if (o->ySpeed < 0) { o->ySpeed = -o->ySpeed; }
            // else { o-ySpeed = 0; }
            o->ySpeed = 0;
            o->y.f.l &= 0x80;
            Object_MoveTowardsLucia(o);
        }
    }

    if (Object_GetMetatile(o) < 0x1f) {
        o->type = OBJ_NONE;
        return;
    }
    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 2;
    Uint16 *frame = (!(o->timer & 8)) ? bospido2 : bospido1;
    if (!Sprite_SetDrawLarge(&spr, o, frame, bospidoOffsets, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }
    spr.size = SPRITE_8X16;
    spr.tile = (!(o->timer & 8)) ? 0x2fa : 0x2ca;
    Sprite_Draw(&spr, o);
    spr.y -= 0x10;
    spr.tile -= 2;
    Sprite_Draw(&spr, o);

    if (o->direction == DIR_RIGHT) { spr.x += 0x1c; }
    else { spr.x -= 0x1c; }
    if (o->timer & 8) { spr.y += 0x10; }
    Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 80);
}