/* bunyon.c: Bunyon object code
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

#include "bunyon.h"
#include "collision.h"
#include "fireball.h"
#include "game.h"
#include "map.h"
#include "object.h"
#include "rng.h"
#include "sprite.h"

static Uint16 bunyonTiles[] = {
    0x2e6, 0x2c6, 0x2c4, 0x2e4, 0x00,
};

static Sint8 bunyonOffsets[] = {
    -0x10,  0x00,
     0x00, -0x10,
     0x10,  0x00,
    -0x08,  0x10,
};

static void Bunyon_HandleMovement(Object *o);
static void Bunyon_HandleSplit(Object *parent, int index, int count);

void Bunyon_InitObj(Object *o) {
    OBJECT_CHECK_SPAWN(o);
    o->hp = 160;
    o->type++;
    o->xSpeed = (o->direction == DIR_RIGHT) ? 0x14 : -0x14;
    o->ySpeed = 0;
    o->timer = 0;
}

void Bunyon_MedInitObj(Object *o) {
    o->hp = 80;
    o->type++;
    o->ySpeed = 0;
    o->timer = 0;
    Object_InitCollision(o);
}

void Bunyon_SmallInitObj(Object *o) {
    o->hp = 32;
    o->type++;
    o->ySpeed = 0;
    o->timer = 0;
    Object_InitCollision(o);
}

void Bunyon_Obj(Object *o) {
    if (!bossActive) {
        goto eraseObj;
    }
    Bunyon_HandleMovement(o);
    
    if (Object_GetMetatile(o) < MAP_SOLID) {
        goto eraseObj;
    }
    Sprite spr = { 0 };
    spr.palette = 2;
    if (!Sprite_SetDrawLarge(&spr, o, bunyonTiles, bunyonOffsets, (o->direction == DIR_LEFT) ? 0 : -16, 0)) {
        goto eraseObj;
    }

    if (!Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 35)) {
        o->type = OBJ_BUNYON_SPLIT;
        o->timer = 0x10;
    }
    return;

eraseObj:
    o->type = OBJ_NONE;
}

void Bunyon_MedObj(Object *o) {
    if (!bossActive) {
        goto eraseObj;
    }
    Bunyon_HandleMovement(o);
    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 2;
    spr.tile = 0x2a6;
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        goto eraseObj;
    }

    if (!Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 18)) {
        o->type = OBJ_MED_BUNYON_SPLIT;
    }
    return;

eraseObj:
    o->type = OBJ_NONE;
}

void Bunyon_SmallObj(Object *o) {
    if (!bossActive) {
        goto eraseObj;
    }
    Bunyon_HandleMovement(o);
    Sprite spr = { 0 };
    spr.size = SPRITE_8X16;
    spr.palette = 2;
    spr.tile = 0x2ac;
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        goto eraseObj;
    }
    Collision_Handle(o, &spr, COLLISION_SIZE_16X16, 12);
    return;

eraseObj:
    o->type = OBJ_NONE;
}

void Bunyon_SplitObj(Object *o) {
    o->timer--;
    if (!o->timer) {
        Bunyon_HandleSplit(o, 0, 4);
        return;
    }

    o->direction = DIR_LEFT;
    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 2;
    spr.tile = 0x2ce;
    if (!Sprite_SetDraw16x32(&spr, o, 0x2cc, 0, 0)) {
        o->type = OBJ_NONE;
        return;
    }
    o->direction = DIR_RIGHT;
    spr.x += 0x10;
    Sprite_DrawDir(&spr, o);
    spr.y += 0x10;
    spr.tile = 0x2ce;
    Sprite_DrawDir(&spr, o);
}

void Bunyon_MedSplitObj(Object *o) {
    o->timer--;
    if (!o->timer) {
        Bunyon_HandleSplit(o, 4, (currObjectIndex == 0x15) ? 2 : 4);
        return;
    }
    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.tile = 0x286;
    if (!Sprite_SetDraw(&spr, o, 0, 0)) {
        o->type = OBJ_NONE;
    }
}

static void Bunyon_HandleMovement(Object *o) {
    o->timer++;
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }
    Object_CheckForWall(o);
    Object_ApplyGravity(o);
    if (Object_UpdateYPos(o)) {
        o->ySpeed = RNG_Get() & 0xe0;
        o->y.f.l &= 0x80;
        Object_MoveTowardsLucia(o);
    }
}

typedef struct {
    Sint16 xOffset;
    Sint16 yOffset;
    Uint8 direction;
    Sint8 xSpeed;
} BUNYON_SPLIT_T;

BUNYON_SPLIT_T bunyonSplits[] = {
    {-0x100, -0x100,  DIR_LEFT,  -0x18},
    {-0x100,   0x00,  DIR_LEFT,  -0x18},
    {  0x00, -0x100, DIR_RIGHT,   0x18},
    {  0x00,   0x00, DIR_RIGHT,   0x18},
    { -0x41,  -0x41,  DIR_LEFT,  -0x1c},
    { -0x41,   0x00,  DIR_LEFT,  -0x1c},
    {  0x00,  -0x41, DIR_RIGHT,   0x1c},
    {  0x00,   0x00, DIR_RIGHT,   0x1c},
};

static void Bunyon_HandleSplit(Object *parent, int index, int count) {
    Object parentBak = *parent;
    // overwrite the parent object with the new bunyon object (this prevents
    // any gaps in the object array so new bunyons won't spawn too often)
    Object *o = parent;
    for (int i = 0; i < count; i++) {
        o->x.v = parentBak.x.v + bunyonSplits[index + i].xOffset;
        o->y.v = parentBak.y.v + bunyonSplits[index + i].yOffset;
        o->direction = bunyonSplits[index + i].direction;
        o->xSpeed = bunyonSplits[index + i].xSpeed;
        o->type = parentBak.type + 1;
        o->collision = parentBak.collision;

        o = Object_FindNext(9, MAX_OBJECTS);
        if (!o) { break; }
    }
    parent->type = OBJ_NONE;
}