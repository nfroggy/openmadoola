/* object.c: object handler & utility functions
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

#include <stdio.h>

#include "biforce.h"
#include "bospido.h"
#include "boundball.h"
#include "bunyon.h"
#include "constants.h"
#include "daltos.h"
#include "dopipu.h"
#include "explosion.h"
#include "eyemon.h"
#include "fire.h"
#include "fireball.h"
#include "flamesword.h"
#include "fountain.h"
#include "gaguzul.h"
#include "game.h"
#include "hopegg.h"
#include "item.h"
#include "joyraima.h"
#include "kikura.h"
#include "lucia.h"
#include "magicbomb.h"
#include "mantleskull.h"
#include "map.h"
#include "nigito.h"
#include "nipata.h"
#include "nishiga.h"
#include "nomaji.h"
#include "nyuru.h"
#include "object.h"
#include "palette.h"
#include "peraskull.h"
#include "shieldball.h"
#include "smasher.h"
#include "spajyan.h"
#include "sprite.h"
#include "suneisa.h"
#include "sword.h"
#include "weapon.h"
#include "weaponfire.h"
#include "wingofmadoola.h"
#include "yokkochan.h"
#include "zadofly.h"

Object objects[MAX_OBJECTS];
int currObjectIndex;

typedef void (*OBJECT_FUNCTION)(Object *o);


OBJECT_FUNCTION objectFunctions[] = {
    NULL,
    Lucia_NormalObj,
    Lucia_ClimbObj,
    Lucia_AirObj,
    Lucia_AirObj,
    MagicBomb_Obj,
    ShieldBall_Obj,
    BoundBall_Obj,
    WeaponFire_MagicBombObj,
    WeaponFire_FlameSwordObj,
    Explosion_Obj,
    Smasher_Obj,
    Sword_Obj,
    FlameSword_Obj,
    Smasher_DamageObj,
    Nomaji_InitObj,
    Nipata_InitObj,
    Dopipu_InitObj,
    Kikura_InitObj,
    Peraskull_InitObj,
    Fire_InitObj,
    MantleSkull_InitObj,
    Zadofly_InitObj,
    Gaguzul_InitObj,
    NULL,
    Spajyan_InitObj,
    Nyuru_InitObj,
    Nishiga_InitObj,
    Eyemon_InitObj,
    YokkoChan_InitObj,
    Hopegg_InitObj,
    Nigito_InitObj,
    Suneisa_InitObj,
    Joyraima_InitObj,
    HyperEyemon_InitObj,
    Biforce_InitObj,
    Bospido_InitObj,
    NULL,
    Daltos_InitObj,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    Nomaji_Obj,
    Nipata_Obj,
    Dopipu_Obj,
    Kikura_Obj,
    Peraskull_Obj,
    Fire_Obj,
    MantleSkull_Obj,
    Zadofly_Obj,
    Gaguzul_Obj,
    NULL,
    Spajyan_Obj,
    Nyuru_Obj,
    Nishiga_Obj,
    Eyemon_Obj,
    YokkoChan_Obj,
    Hopegg_Obj,
    Nigito_Obj,
    Suneisa_Obj,
    Joyraima_Obj,
    NULL,
    Biforce_Obj,
    Bospido_Obj,
    NULL,
    Daltos_Obj,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    Lucia_LvlEndDoorObj,
    Lucia_WarpDoorObj,
    Item_Obj,
    Bunyon_InitObj,
    Bunyon_Obj,
    Bunyon_SplitObj,
    Bunyon_MedInitObj,
    Bunyon_MedObj,
    Bunyon_MedSplitObj,
    Bunyon_SmallInitObj,
    Bunyon_SmallObj,
    Fountain_Obj,
    Lucia_DyingObj,
    WingOfMadoola_Obj,
    Fireball_Obj,
};

#define ENEMY_SLOT (9)

void Object_ListInit(void) {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        objects[i].type = OBJ_NONE;
    }
}

Object *Object_FindNext(int min, int max) {
    for (int i = min; i < max; i++) {
        if (objects[i].type == OBJ_NONE) {
            return &objects[i];
        }
    }

    return NULL;
}

void Object_ListRun(void) {
    for (currObjectIndex = 0; currObjectIndex < MAX_OBJECTS; currObjectIndex++) {
        Uint8 type = objects[currObjectIndex].type;
        if (type) {
            if ((type >= NUM_OBJECTS) || (objectFunctions[type] == NULL)) {
                printf("Object 0x%X not implemented\n", type);
                objects[currObjectIndex].type = OBJ_NONE;
            }
            else {
                objectFunctions[type](&objects[currObjectIndex]);
            }
        }
    }
}

void Object_DeleteRange(int start) {
    for (int i = start; i < MAX_OBJECTS; i++) {
        objects[i].type = OBJ_NONE;
    }
}

void Object_DeleteAllAfterLucia(void) {
    // Lucia goes in object slot 0
    Object_DeleteRange(1);
}

void Object_InitCollision(Object *o) {
    o->collision = o->y.f.h * roomWidthMetatiles + o->x.f.h;
}

void Object_DecCollisionX(Object *o) {
    o->collision--;
}

void Object_IncCollisionX(Object *o) {
    o->collision++;
}

void Object_DecCollisionY(Object *o) {
    o->collision -= roomWidthMetatiles;
}

void Object_IncCollisionY(Object *o) {
    o->collision += roomWidthMetatiles;
}

int Object_CollisionOutOfBounds(Object *o) {
    return o->collision >= roomWidthMetatiles * roomHeightMetatiles;
}

void Object_SetDirection(Object *o) {
    if (o->xSpeed > 0) {
        o->direction = DIR_RIGHT;
    }
    else if (o->xSpeed < 0) {
        o->direction = DIR_LEFT;
    }
}

void Object_TurnAround(Object *o) {
    o->xSpeed = -o->xSpeed;
    Object_SetDirection(o);
}

Uint16 Object_GetMetatile(Object *o) {
    return mapMetatiles[o->collision];
}

Uint16 Object_GetMetatileBelow(Object *o) {
    return mapMetatiles[o->collision + roomWidthMetatiles];
}

void Object_MetatileAlignX(Object *o) {
    o->x.f.l = 0x80;
}

void Object_MetatileAlignY(Object *o) {
    o->y.f.l = 0x80;
}

int Object_UpdateXPos(Object *o) {
    Fixed16 oldX = o->x;
    o->x.v += o->xSpeed;
    // if we're on the same metatile we were previously on, check if there's a
    // solid tile next to us
    if (oldX.f.h == o->x.f.h) {
        return Map_CheckX(o);
    }

    // otherwise, since we were allowed to move onto this tile we should update
    // our collision position
    if (o->xSpeed >= 0) { Object_IncCollisionX(o); }
    else { Object_DecCollisionX(o); }

    return 0;
}

int Object_UpdateYPos(Object *o) {
    Fixed16 oldY = o->y;
    o->y.v += o->ySpeed;
    // if we're on the same metatile we were previously on, check if there's a
    // solid tile next to us
    if (oldY.f.h == o->y.f.h) {
        return Map_CheckY(o);
    }

    // otherwise, since we were allowed to move onto this tile we should update
    // our collision position
    if (o->ySpeed >= 0) { Object_IncCollisionY(o); }
    else { Object_DecCollisionY(o); }
    
    return 0;
}

void Object_CheckForWall(Object *o) {
    if (Object_UpdateXPos(o)) {
        Object_TurnAround(o);
    }
}

void Object_CheckForDrop(Object *o) {
    // if the metatile below the object isn't either solid ground or a ladder,
    // make the object turn around
    if (Object_GetMetatileBelow(o) >= MAP_LADDER) {
        Object_TurnAround(o);
    }
}

void Object_Bounce(Object *o) {
    if (Object_UpdateXPos(o)) {
        o->xSpeed = -o->xSpeed;
    }

    if (Object_UpdateYPos(o)) {
        o->ySpeed = -o->ySpeed;
    }
}

void Object_CalcXPos(Object *o) {
    Fixed16 oldX = o->x;
    o->x.v += o->xSpeed;

    // if we've changed metatiles, update the collision position
    if (o->x.f.h > oldX.f.h) { Object_IncCollisionX(o); }
    else if (o->x.f.h < oldX.f.h) { Object_DecCollisionX(o); }
}

void Object_CalcYPos(Object *o) {
    Fixed16 oldY = o->y;
    o->y.v += o->ySpeed;

    // if we've changed metatiles, update the collision position
    if (o->y.f.h > oldY.f.h) { Object_IncCollisionY(o); }
    else if (o->y.f.h < oldY.f.h) { Object_DecCollisionY(o); }
}

void Object_CalcXYPos(Object *o) {
    Object_CalcXPos(o);
    Object_CalcYPos(o);
}

int Object_TouchingGround(Object *o) {
    // if we're in the upper half of the tile, don't bother checking if we're
    // touching the ground or not
    if (o->y.f.l < 0x80) {
        return 0;
    }

    // if the object position is less than 5.5 pixels into the metatile, look at
    // the previous and current metatiles
    if (o->x.f.l < 0x58) {
        if (Map_SolidTileBelow(o->collision - 1)) { goto foundSolidTile; }
    }

    if (Map_SolidTileBelow(o->collision)) { goto foundSolidTile; }

    // if the object position is more than 10.5 pixels into the metatile, look
    // at the current and next metatiles
    if (o->x.f.l >= 0xa8) {
        if (Map_SolidTileBelow(o->collision + 1)) { goto foundSolidTile; }
    }

    return 0;

foundSolidTile:
    // align y pos to metatiles
    o->y.f.l = 0x80;
    return 1;
}

void Object_ApplyGravity(Object *o) {
    // check for overflow (need to do it before adding to avoid undefined behavior)
    // 127 because o->ySpeed is a signed 8-bit int
    if ((127 - o->ySpeed) < 9) {
        o->ySpeed = 127;
    }
    else {
        o->ySpeed += 9;
    }
}

void Object_FaceLucia(Object *o) {
    // face right if the object is behind lucia
    if (o->x.v < objects[0].x.v) {
        o->direction = DIR_RIGHT;
    }
    // face left if the object is in front of lucia
    else {
        o->direction = DIR_LEFT;
    }
}

void Object_MoveTowardsLucia(Object *o) {
    // make object turn around if it's behind Lucia and moving left
    if (o->x.v < objects[0].x.v) {
        if (o->xSpeed < 0) {
            Object_TurnAround(o);
        }
    }

    // make object turn around if it's in front of Lucia and moving right
    else {
        if (o->xSpeed >= 0) {
            Object_TurnAround(o);
        }
    }
}

int Object_PutOnGround(Object *o) {
    for (int i = 0; i < 16; i++) {
        // if we've gone past the bounds of the level array, bail out
        if (o->collision >= (roomWidthMetatiles * roomHeightMetatiles)) {
            o->type = OBJ_NONE;
            return 0;
        }

        Uint16 metatile = Object_GetMetatile(o);
            // if the object is in a scenery metatile and the metatile below the
            // object is solid, we've successfully put the object on the ground
            if (metatile >= MAP_LADDER) {
                if (Object_GetMetatileBelow(o) < MAP_LADDER) {
                    o->x.f.l = 0x80;
                    o->y.f.l = 0x80;
                    return 1;
                }
            }

        o->y.f.h++;
        Object_IncCollisionY(o);
    }

    // the entire column is either all air or all solid, so we can't put the
    // object on the ground
    o->type = OBJ_NONE;
    return 0;
}

int Object_HandleJump(Object *o) {
    // return if not in the air
    if (o->ySpeed == 0) {
        return 0;
    }

    Object_ApplyGravity(o);
    if (o->ySpeed > 0) {
        Object_CheckForWall(o);
    }

    // if the object hit the ground, make its y speed 0 and align it to the metatile grid
    if (Object_UpdateYPos(o)) {
        o->ySpeed = 0;
        o->y.f.l &= 0x80;
    }

    return 1;

}

void Object_LimitDistance(Object *o) {
    Sint16 difference = o->x.v - luciaXPos.v;
    // object to the left of lucia
    if (difference < 0) {
        difference = -difference;
        // limit distance to just under 6 metatiles
        if (difference >= 0x600) {
            o->x.v = luciaXPos.v - 0x5ff;
        }
    }
    // object to the right of lucia
    else {
        // limit distance to just under 6 metatiles
        if (difference >= 0x600) {
            o->x.v = luciaXPos.v + 0x5ff;
        }
    }
}

void Object_JumpIfHitWall(Object *o) {
    if (Object_UpdateXPos(o)) {
        o->ySpeed = 0x80;
    }
}