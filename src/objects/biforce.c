/* biforce.c: Biforce object code
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

#include "biforce.h"
#include "collision.h"
#include "fireball.h"
#include "lucia.h"
#include "map.h"
#include "sprite.h"
#include "object.h"

static void Biforce_StandingObj(Object *o);
static void Biforce_CrawlingObj(Object *o);

void Biforce_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->hp = 255;
    o->type += 0x20;
    o->timer = 0x54;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x10 : -0x10;
    o->ySpeed = 0;
}

void Biforce_Obj(Object *o) {
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }

    if (o->hp >= 128) {
        Biforce_StandingObj(o);
    }
    else {
        Biforce_CrawlingObj(o);
    }
}

static Sint8 biforceStandingOffsets[] = {
     0x00, -0x10,
     0x00, -0x10,
     0x10,  0x00,
     0x00,  0x10,
    -0x1c, -0x10,
};

static Uint16 biforceStanding1[] = {
    0x294, 0x292, 0x290, 0x2b0, 0x2b2, 0x00,
};

static Uint16 biforceStanding2[] = {
    0x294, 0x292, 0x290, 0x2e0, 0x2e2, 0x00,
};

static void Biforce_StandingObj(Object *o) {
    if (!(o->timer & 0x80)) {
        if (o->timer & 0x40) {
            Object_CheckForWall(o);
            o->timer--;
            if (!(o->timer & 0x3f)) {
                Object_MoveTowardsLucia(o);
                o->timer = 0x3f;
                o->xSpeed = (o->direction == DIR_RIGHT) ? -0x10 : 0x10;
            }
            else {
                o->xSpeed += (o->direction == DIR_RIGHT) ? 0x4 : -0x4;
            }
        }
        else {
            Object_CheckForWall(o);
            o->timer--;
            if (!(o->timer & 0x3f)) {
                o->timer = 0xbf;
            }
        }
    }
    else {
        Fireball_Spawn(0xf, o);
        Object_MoveTowardsLucia(o);
        o->timer--;
        if (!(o->timer & 0x3f)) {
            o->timer = 0x54;
        }
    }

    if (Object_GetMetatile(o) < MAP_SOLID) {
        o->type = OBJ_NONE;
        return;
    }
    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 2;
    Uint16 *frame = (!(o->timer & 8)) ? biforceStanding1 : biforceStanding2;
    if (!Sprite_SetDrawLarge(&spr, o, frame, biforceStandingOffsets, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }

    spr.size = SPRITE_8X16;
    spr.tile = (!(o->timer & 8)) ? 0x280 : 0x2d0;
    Sprite_Draw(&spr, o);
    spr.tile += 2;
    spr.y += 0x10;
    Sprite_Draw(&spr, o);
    spr.y += 0x10;
    spr.x += (o->direction == DIR_RIGHT) ? -0x18 : 0x18;
    if (!(o->timer & 0x4)) {
        spr.tile = 0x2b4;
    }
    else {
        spr.tile = 0x284;
    }
    Sprite_Draw(&spr, o);
    spr.y -= 0x20;
    spr.x += (o->direction == DIR_RIGHT) ? 0xc : -0x1c;
    Collision_Handle(o, &spr, COLLISION_SIZE_16X32, 80);
}

static Sint8 biforceCrawlingOffsets[] = {
     0x00, -0x10,
     0x10,  0x00,
     0x00,  0x10,
    -0x1c,  0x00,
};

static Uint16 biforceCrawling1[] = {
    0x2ae, 0x290, 0x2b0, 0x2b2, 0x00,
};

static Uint16 biforceCrawling2[] = {
    0x2ae, 0x290, 0x2e0, 0x2e2, 0x00,
};

static Sint8 Biforce_CrawlingSpeed(Fixed16 pos, Fixed16 luciaPos) {
    Sint8 difference = ABS(pos.f.h - luciaPos.f.h);
    Sint8 speed = (Sint8)((difference << 4));
    if (speed < 0x10) { speed = 8; }
    return speed;
}

static void Biforce_CrawlingObj(Object *o) {
    if (!(o->timer & 0x80)) {
        if (!(o->timer & 0x40)) {
            Fireball_Spawn(0xf, o);
            o->ySpeed = Biforce_CrawlingSpeed(o->y, luciaYPos);
            if (o->y.f.h >= luciaYPos.f.h) { o->ySpeed = -o->ySpeed; }
            o->xSpeed = Biforce_CrawlingSpeed(o->x, luciaXPos);
            if (o->direction == DIR_LEFT) { o->xSpeed = -o->xSpeed; }
            o->timer--;
            if (!o->timer) { o->timer = 0x68; }
            Object_MoveTowardsLucia(o);
            Object_CalcXYPos(o);
        }
        else {
            o->xSpeed = (o->direction == DIR_RIGHT) ? -0x20 : 0x20;
            o->ySpeed = -0x18;
            o->timer--;
            if (!(o->timer & 0x3f)) {
                o->xSpeed = -o->xSpeed;
                o->timer = 0xc0;
            }
            Object_CalcXYPos(o);
        }
    }
    else {
        o->timer--;
        if (!(o->timer & 0x7f)) {
            o->timer = 0x30;
        }
        Object_MoveTowardsLucia(o);
    }

    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 2;
    Object_LimitDistance(o);
    Uint16 *frame = (!(o->x.f.h & 1)) ? biforceCrawling1 : biforceCrawling2;
    if (!Sprite_SetDrawLarge(&spr, o, frame, biforceCrawlingOffsets, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }
    spr.size = SPRITE_8X16;
    spr.tile = (!(o->x.f.h & 1)) ? 0x282 : 0x2d2;
    Sprite_Draw(&spr, o);
    spr.tile -= 2;
    spr.y -= 0x10;
    Sprite_Draw(&spr, o);

    spr.x += (o->direction == DIR_RIGHT) ? -0xc : 0xc;
    Collision_Handle(o, &spr, COLLISION_SIZE_16X32, 80);
}