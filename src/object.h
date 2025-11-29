/* object.h: object handler & utility functions
 * Copyright (c) 2023, 2025 Nathan Misner
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

// --- lucia gameplay objects ---
#define OBJ_NONE		             (0x0)
#define OBJ_LUCIA_NORMAL	      (0x1)
#define OBJ_LUCIA_CLIMB		      (0x2)
#define OBJ_LUCIA_AIR_LOCKED	  (0x3)
#define OBJ_LUCIA_AIR		        (0x4)
// --- weapon objects ---
#define OBJ_MAGIC_BOMB		       (0x5)
#define OBJ_SHIELD_BALL		      (0x6)
#define OBJ_BOUND_BALL		       (0x7)
#define OBJ_MAGIC_BOMB_FIRE    (0x8)
#define OBJ_FLAME_SWORD_FIRE	  (0x9)
#define OBJ_EXPLOSION		        (0xA)
#define OBJ_SMASHER		          (0xB)
#define OBJ_SWORD		            (0xC)
#define OBJ_FLAME_SWORD	       (0xD)
#define OBJ_SMASHER_DAMAGE	    (0xE)
// --- enemy initialization objects ---
#define OBJ_NOMAJI_INIT		      (0xF)
#define OBJ_NIPATA_INIT		      (0x10)
#define OBJ_DOPIPU_INIT		      (0x11)
#define OBJ_KIKURA_INIT		      (0x12)
#define OBJ_PERASKULL_INIT	    (0x13)
#define OBJ_FIRE_INIT		        (0x14)
#define OBJ_MANTLE_SKULL_INIT	 (0x15)
#define OBJ_ZADOFLY_INIT	      (0x16)
#define OBJ_GAGUZUL_INIT	      (0x17)
#define OBJ_SPAJYAN_INIT	      (0x19)
#define OBJ_NYURU_INIT		       (0x1A)
#define OBJ_NISHIGA_INIT	      (0x1B)
#define OBJ_EYEMON_INIT		      (0x1C)
#define OBJ_YOKKO_CHAN_INIT	   (0x1D)
#define OBJ_HOPEGG_INIT		      (0x1E)
#define OBJ_NIGITO_INIT		      (0x1F)
#define OBJ_SUNEISA_INIT	      (0x20)
#define OBJ_JOYLIMER_INIT	     (0x21)
#define OBJ_HYPER_EYEMON_INIT	 (0x22)
#define OBJ_BIFORCE_INIT	      (0x23)
#define OBJ_BOSPIDO_INIT	      (0x24)
#define OBJ_DALTOS_INIT	       (0x26)
// --- enemy objects ---
#define OBJ_NOMAJI		           (0x2F)
#define OBJ_NIPATA		           (0x30)
#define OBJ_DOPIPU		           (0x31)
#define OBJ_KIKURA		           (0x32)
#define OBJ_PERASKULL		        (0x33)
#define OBJ_FIRE		             (0x34)
#define OBJ_MANTLE_SKULL	      (0x35)
#define OBJ_ZADOFLY		          (0x36)
#define OBJ_GAGUZUL		          (0x37)
#define OBJ_SPAJYAN		          (0x39)
#define OBJ_NYURU		            (0x3A)
#define OBJ_NISHIGA		          (0x3B)
#define OBJ_EYEMON		           (0x3C)
#define OBJ_YOKKO_CHAN		       (0x3D)
#define OBJ_HOPEGG		           (0x3E)
#define OBJ_NIGITO		           (0x3F)
#define OBJ_SUNEISA		          (0x40)
#define OBJ_JOYLIMER	    	     (0x41)
#define OBJ_BIFORCE		          (0x43)
#define OBJ_BOSPIDO		          (0x44)
#define OBJ_DALTOS		           (0x46)
// --- misc objects (and bunyon) ---
#define OBJ_LUCIA_LVL_END_DOOR	(0x4F)
#define OBJ_LUCIA_WARP_DOOR	   (0x50)
#define OBJ_ITEM		             (0x51)
#define OBJ_BUNYON_INIT		      (0x52)
#define OBJ_BUNYON		           (0x53)
#define OBJ_BUNYON_SPLIT	      (0x54)
#define OBJ_MED_BUNYON_INIT	   (0x55)
#define OBJ_MED_BUNYON		       (0x56)
#define OBJ_MED_BUNYON_SPLIT	  (0x57)
#define OBJ_SMALL_BUNYON_INIT	 (0x58)
#define OBJ_SMALL_BUNYON	      (0x59)
#define OBJ_FOUNTAIN		         (0x5A)
#define OBJ_LUCIA_DYING		      (0x5B)
#define OBJ_WING_OF_MADOOLA	   (0x5C)
#define OBJ_FIREBALL		         (0x5D)
#define NUM_OBJECTS            (0x5E)

#define DIR_LEFT 0x80
#define DIR_RIGHT 0

typedef struct {
    Uint8 direction; // nonzero = facing left, zero = facing right
    Uint8 stunnedTimer;
    Sint16 hp;
    Fixed16 x;
    Fixed16 y;
    Uint16 collision;
    Sint8 xSpeed;
    Sint8 ySpeed;
    Uint8 timer;
    Uint8 type;
} Object;

#define MAX_OBJECTS (256)
// object 0 = Lucia
// objects 1-8 = Lucia's weapons
// objects 9-MAX_OBJECTS: anything else
extern Object objects[MAX_OBJECTS];

// The object currently being run by Object_ListRun
extern int currObjectIndex;

/**
 * @brief Clears the object list.
*/
void Object_ListInit(void);

/**
 * @brief Gets the next free object within the specified bounds
 * @param min the low index to search from
 * @param max the high index to search from
 * @return the pointer to the found object, or NULL if there aren't any free
*/
Object *Object_FindNext(int min, int max);

/**
 * @brief runs the object code for each object in the list
*/
void Object_ListRun(void);

/**
 * @brief Deletes the given index and all objects after it.
 * @param start The index to start deletion at
*/
void Object_DeleteRange(int start);

/**
 * @brief deletes all onscreen objects besides the one in slot 0 (Lucia)
*/
void Object_DeleteAllAfterLucia(void);

/**
 * @brief Sets up an object's collision offset.
*/
void Object_InitCollision(Object *o);

/**
 * @brief Checks if an object's collision variable is pointing off the map
 * @param o object to check
 * @returns nonzero if the object's collision is out of the map bounds, zero otherwise
 */
int Object_CollisionOutOfBounds(Object *o);

/**
 * @brief Sets an object's direction variable from its x speed.
*/
void Object_SetDirection(Object *o);

/**
 * @brief Negates an object's x speed and direction.
*/
void Object_TurnAround(Object *o);

/**
 * @brief Gets the metatile an object is currently touching.
 * @returns the metatile number
*/
Uint16 Object_GetMetatile(Object *o);

/**
 * @brief Aligns an object's x position to the metatile grid.
*/
void Object_MetatileAlignX(Object *o);

/**
 * @brief Aligns an object's y position to the metatile grid.
*/
void Object_MetatileAlignY(Object *o);

/**
 * @brief Sets an object's x position based on its speed, handles collision
 * @returns nonzero if the object hit something during its movement.
*/
int Object_UpdateXPos(Object *o);

/**
 * @brief Sets an object's y position based on its speed, handles collision
 * @returns nonzero if the object hit something during its movement.
*/
int Object_UpdateYPos(Object *o);

/**
 * @brief Updates an object's x position, then makes it turn around if it hit a wall.
*/
void Object_CheckForWall(Object *o);

/**
 * @brief Makes an object turn around if the metatile below it isn't ground
*/
void Object_CheckForDrop(Object *o);

/**
 * @brief Updates an object's x and y position, and inverts the x and/or y speed if there's
 * a collision with the background
 * @param o The object to update
*/
void Object_Bounce(Object *o);

/**
 * @brief Sets an object's x position based on its speed, doesn't handle collision
*/
void Object_CalcXPos(Object *o);


/**
 * @brief Sets an object's y position based on its speed, doesn't handle collision
*/
void Object_CalcYPos(Object *o);

/**
 * @brief Sets an object's x and y positions based on its speed, doesn't handle collision
*/
void Object_CalcXYPos(Object *o);

/**
 * @returns nonzero if the object is touching the ground
*/
int Object_TouchingGround(Object *o);

/**
 * @brief Adds the gravity acceleration constant (9/16 of a pixel) to an object's y speed.
*/
void Object_ApplyGravity(Object *o);

/**
 * @brief Sets the object's direction variable to point towards Lucia
*/
void Object_FaceLucia(Object *o);

/**
 * @brief Sets the object's speed variable to move towards Lucia
*/
void Object_MoveTowardsLucia(Object *o);

/**
 * @brief Tries to put the object on top of a solid metatile. When being used in object
 * initialization code, the OBJECT_CHECK_SPAWN macro may be helpful.
 * @param o The object to update
 * @returns nonzero on success, on failure returns zero and sets the object's type to OBJ_NONE
*/
int Object_PutOnGround(Object *o);
#define OBJECT_CHECK_SPAWN(o) do { if (!Object_PutOnGround(o)) { return; } } while (0)

/**
 * @brief Handles an object's jumping movement and collision
 * @param o The object to update
 * @returns 0 if the object isn't in the air, nonzero if it is
*/
int Object_HandleJump(Object *o);

/**
 * @brief Keeps the object's x position within 6 metatiles of Lucia's
 * @param o The object to update
*/
void Object_LimitDistance(Object *o);

/**
 * @brief Handles an object's x movement, makes it jump if it hit a wall
 * @param o The object to update
*/
void Object_JumpIfHitWall(Object *o);
