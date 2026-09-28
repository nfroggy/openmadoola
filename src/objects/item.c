/* item.c: Item pickup object code
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

#include <string.h>

#include "collision.h"
#include "constants.h"
#include "game.h"
#include "item.h"
#include "map.h"
#include "object.h"
#include "sprite.h"

Uint8 itemsCollected[8192];

Uint16 itemTiles[] = {
    0x60,   // regular sword
    0x60,   // flame sword
    0x66,   // magic bomb
    0x62,   // bound ball
    0x64,   // shield ball
    0x68,   // smasher
    0x6a,   // flash
    0x2bc,  // boots
    0x68a,  // apple
    0x688,  // pot
    0x6aa,  // scroll
    0x6a8,  // spellbook
    0x2e,   // red potion (recovers hp)
    0x2e,   // purple potion (increases max hp)
    0x2e,   // blue potion (recovers mp)
    0x2e,   // yellow potion (increases max mp)
    0x68c,  // orb
};

// orb palette gets set by the frame counter so it's not here
Uint8 itemPalettes[] = {
    1,  // regular sword
    3,  // flame sword
    3,  // magic bomb
    1,  // bound ball
    3,  // shield ball
    3,  // smasher
    1,  // flash
    0,  // boots
    0,  // apple
    1,  // pot
    3,  // scroll
    1,  // spellbook
    0,  // red potion (recovers hp)
    1,  // purple potion (increases max hp)
    2,  // blue potion (recovers mp)
    3,  // yellow potion (increases max mp)
};

static void Item_SetCollected(Object *o);

void Item_Obj(Object *o) {
    o->direction = DIR_LEFT;
    o->stunnedTimer = 0;
    // hp variable indicates the powerup type
    Sprite spr = { 0 };
    // the hp variable gets used to indicate the powerup type
    spr.tile = itemTiles[o->hp];

    // set up sprite attributes
    if (o->hp == ITEM_ORB) {
        spr.mirror = gameFrames & 3;
        spr.palette = gameFrames & 3;
        spr.size = SPRITE_16X16;
    }
    else {
        spr.mirror = 0;
        spr.palette = itemPalettes[o->hp];
    }

    if (o->hp < 8) {
        spr.size = SPRITE_8X16;
    }
    else {
        spr.size = SPRITE_16X16;
    }

    // Calling Sprite_SetDraw on the object (which the original game did the
    // equivalent of) erases the horizontal mirroring on the orb, which make it
    // look less cool in my opinion. I'm pretty sure this is a bug.
    if (gameType != GAME_TYPE_ORIGINAL) {
        if (!Sprite_SetPos(&spr, o, 0, 0)) {
            goto eraseItem;
        } else {
            Sprite_Draw(&spr, NULL);
        }
    }
    else {
        if (!Sprite_SetDraw(&spr, o, 0, 0)) {
            goto eraseItem;
        }
    }

    // if the item is a potion, it spawns outside an item room so give it gravity
    if ((o->hp >= ITEM_RED_POTION) && (o->hp <= ITEM_YELLOW_POTION)) {
        if (!Object_TouchingGround(o)) {
            Object_ApplyGravity(o);
            Object_UpdateYPos(o);
        }
    }

    // if lucia picked up the item, mark as collected
    if (Collision_WithLucia(o, &spr, COLLISION_SIZE_16X16, o->hp + ITEM_FLAG) == 2) {
        if (currRoom == 15) {
            Item_SetCollected(o);
        }
    }
    else {
        return;
    }

eraseItem:
    o->type = OBJ_NONE;
}

void Item_InitCollected(void) {
    memset(itemsCollected, 0, sizeof(itemsCollected));
}

static void Item_SetCollected(Object *o) {
    int idBit = o->id & 7;
    int idByte = o->id >> 3;
    itemsCollected[idByte] |= (1 << idBit);
}

Uint8 Item_Collected(Uint16 id) {
    int idBit = id & 7;
    int idByte = id >> 3;
    return itemsCollected[idByte] & (1 << idBit);
}
