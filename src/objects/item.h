/* item.h: Item pickup object code
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

#pragma once
#include "constants.h"
#include "object.h"

typedef enum {
    ITEM_SWORD = 0,
    ITEM_FLAME_SWORD,
    ITEM_MAGIC_BOMB,
    ITEM_BOUND_BALL,
    ITEM_SHIELD_BALL,
    ITEM_SMASHER,
    ITEM_FLASH,
    ITEM_BOOTS,
    ITEM_APPLE,
    ITEM_POT,
    ITEM_SCROLL,
    ITEM_SPELLBOOK,
    ITEM_RED_POTION,
    ITEM_PURPLE_POTION,
    ITEM_BLUE_POTION,
    ITEM_YELLOW_POTION,
    ITEM_ORB,
} ITEM_TYPE;

// the flag added to the "object damage" variable to tell Lucia's code she's touching an item
#define ITEM_FLAG (0xA0)

extern Uint8 itemsCollected[8192];

/**e
 * @brief item object code
 * @param o object pointer
*/
void Item_Obj(Object *o);

/**
 * @brief Sets all of the item room items to "not collected"
*/
void Item_InitCollected(void);

/**
 * @brief checks if an item was collected
 * @param id the id to check for collection status
 * @returns nonzero if the item was collected, zero otherwise
*/
Uint8 Item_Collected(Uint16 id);
