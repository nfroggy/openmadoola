/* joylimer.c: Joylimer object code
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
#include "joylimer.h"

#include "map.h"
#include "object.h"
#include "sound.h"
#include "sprite.h"

void Joylimer_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    Sound_Play(SFX_JOYLIMER);
    o->hp = 192;
    o->type += 0x20;
    o->timer = 0;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x4 : -0x4;
    o->ySpeed = 0;
}

static Sint8 joylimerXOffsets[] = {
    0x10, -0x10, -0x10, 0x10
};

void Joylimer_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    o->timer++;
    if (o->direction == DIR_RIGHT) {
        o->xSpeed += 0x1;
        o->xSpeed = MIN(o->xSpeed, 0x38);
    }
    else {
        o->xSpeed -= 0x1;
        o->xSpeed = MAX(o->xSpeed, -0x38);
    }
    Object_FaceLucia(o);
    Object_CheckForWall(o);
    if (!Object_TouchingGround(o)) {
        o->xSpeed = -o->xSpeed;
        o->direction ^= 0x80;
    }

    if (Object_GetMetatile(o) < MAP_SOLID) {
        o->type = OBJ_NONE;
        return;
    }
    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 0;
    if (!(o->timer & 4)) {
        spr.tile = 0x186;
    }
    else {
        spr.tile = 0x1c6;
    }
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }
    spr.y -= 0x10;
    spr.tile -= 2;
    Sprite_Draw(&spr, o);
    Uint8 xCursor = (o->direction == DIR_RIGHT) ? 2 : 0;
    spr.x += joylimerXOffsets[xCursor];
    xCursor++;
    spr.tile += 0x20;
    Sprite_Draw(&spr, o);
    spr.y += 0x10;
    spr.tile += 2;
    Sprite_Draw(&spr, o);
    spr.y -= 0x10;
    spr.x += joylimerXOffsets[xCursor];
    Collision_Handle(o, &spr, COLLISION_SIZE_16X32, 60);
}