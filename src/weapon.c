/* weapon.c: Weapon variables and object code
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

#include "constants.h"
#include "game.h"
#include "joy.h"
#include "lucia.h"
#include "map.h"
#include "palette.h"
#include "sound.h"
#include "weapon.h"

Uint8 weaponLevels[NUM_WEAPONS];
Uint8 currentWeapon;
Uint8 weaponDamage;

static Uint8 weaponDamageTbl[] = {
    0x01, 0x0A, 0x14, // sword
    0x05, 0x0A, 0x14, // flame sword
    0x02, 0x08, 0x0A, // magic bomb
    0x05, 0x0A, 0x14, // bound ball
    0x01, 0x01, 0x01, // shield ball
    0x32, 0x50, 0x64, // smasher
    0x01, 0x02, 0x03, // flash
};

static Uint16 weaponMagicTbl[] = {
    0,      // sword
    10,     // flame sword
    20,     // magic bomb
    10,     // bound ball
    150,    // shield ball
    100,    // smasher
    500,    // flash
};

WeaponCoords weaponCoords[MAX_WEAPONS];

static void Weapon_InitSword(void);
static void Weapon_InitFlameSword(void);
static void Weapon_InitMagicBomb(void);
static void Weapon_InitBoundBall(void);
static void Weapon_InitShieldBall(void);
static void Weapon_InitSmasher(void);
static void Weapon_InitFlash(void);

void Weapon_Init(void) {
    // erase all weapon objects
    for (int i = 1; i < 9; i++) {
        objects[i].type = OBJ_NONE;
    }

    // set all weapon coordinates to "not spawned"
    for (int i = 0; i < MAX_WEAPONS; i++) {
        weaponCoords[i].spawned = 0;
        weaponCoords[i].collided = 0;
    }
}

void Weapon_Process(void) {
    // can't use weapons if we're going to another room
    if (roomChangeTimer) {
        return;
    }

    // check if B is pressed
    if (joyEdge & JOY_B) {
        Uint8 damageOffset = (currentWeapon * 3) + (weaponLevels[currentWeapon] - 1);
        weaponDamage = weaponDamageTbl[damageOffset];

        switch (currentWeapon) {
        case WEAPON_SWORD:
            Weapon_InitSword();
            break;

        case WEAPON_FLAME_SWORD:
            Weapon_InitFlameSword();
            break;

        case WEAPON_MAGIC_BOMB:
            Weapon_InitMagicBomb();
            break;

        case WEAPON_BOUND_BALL:
            Weapon_InitBoundBall();
            break;

        case WEAPON_SHIELD_BALL:
            Weapon_InitShieldBall();
            break;

        case WEAPON_SMASHER:
            Weapon_InitSmasher();
            break;

        case WEAPON_FLASH:
            Weapon_InitFlash();
            break;
        }
    }
}

Sint16 Weapon_MagicAfterUse(void) {
    return magic - weaponMagicTbl[currentWeapon];
}

static void Weapon_SubtractMagic(void) {
    magic = Weapon_MagicAfterUse();
    if (magic < 0) { magic = 0; }
    if (Weapon_MagicAfterUse() < 0) {
        // if there's not enough magic to use the weapon again, switch to the basic sword
        currentWeapon = WEAPON_SWORD;
    }
}

static void Weapon_InitSword(void) {
    if (attackTimer) {
        return;
    }

    attackTimer = 11;
    objects[1].type = OBJ_SWORD;
    Weapon_SubtractMagic();
    Sound_Play(SFX_SWORD);
}

static void Weapon_InitFlameSword(void) {
    if (attackTimer) {
        return;
    }

    // spawn sword object
    attackTimer = 11;
    objects[1].type = OBJ_SWORD;

    // spawn flame object
    if (objects[2].type == OBJ_NONE) {
        objects[2].x = luciaXPos;
        objects[2].y = luciaYPos;
        if (!(joy & JOY_DOWN)) {
            objects[2].y.f.h--;
        }
        objects[2].x.f.h++;
        objects[2].xSpeed = 0x60;
        // handle facing left
        if (objects[0].direction) {
            objects[2].x.f.h -= 2;
            objects[2].xSpeed = -objects[2].xSpeed;
        }
        // handle holding up
        if (joy & JOY_UP) {
            objects[2].ySpeed = 0xD0;
        }
        else {
            objects[2].ySpeed = 0;
        }
        objects[2].type = OBJ_FLAME_SWORD;
        objects[2].timer = 8;
    }
    Weapon_SubtractMagic();
    Sound_Play(SFX_FLAME_SWORD);
}

static void Weapon_InitMagicBomb(void) {
    // can't have multiple concurrent magic bombs 
    if (objects[1].type) {
        return;
    }

    // copy lucia's object into magic bomb object slot
    objects[1] = objects[0];

    // set up position
    objects[1].y.v = objects[0].y.v - 0x80;
    objects[1].type = OBJ_MAGIC_BOMB;
    objects[1].timer = 3;
    // if lucia is facing right
    if (objects[1].direction == 0) {
        objects[1].xSpeed = 0x48;
    }
    else {
        objects[1].xSpeed = 0xB8;
    }
    objects[1].ySpeed = 0;
    Weapon_SubtractMagic();
    Sound_Play(SFX_BOMB);
}

static void Weapon_InitBoundBall(void) {

    Object *o = Object_FindNext(1, 9);
    if (o == NULL) {
        return;
    }

    // copy lucia's object into bound ball object slot
    *o = objects[0];

    if (objects[0].type == OBJ_LUCIA_NORMAL) {
        // handle ducking
        if (joyDir != 5) {
            o->y.f.h--;
        }
    }
    else {
        o->y.f.h--;
    }

    if (joy & JOY_UP) {
        o->ySpeed = 0x81;
        o->xSpeed = (gameFrames & 0x3f) - 0x20;
    }
    else {
        if (objects[0].direction) {
            o->xSpeed = 0x81;
        }
        else {
            o->xSpeed = 0x7f;
        }

        o->ySpeed = (gameFrames & 0x3f) - 0x20;
    }

    o->type = OBJ_BOUND_BALL;
    o->timer = 0x64;
    Weapon_SubtractMagic();
    Sound_Play(SFX_BOUND_BALL);
}

static Uint8 shieldBallTimerTbl[] = {
    24, 35, 50,
};

static void Weapon_InitShieldBall(void) {
    Uint8 timerVal = shieldBallTimerTbl[weaponLevels[WEAPON_SHIELD_BALL] - 1];

    for (int i = 8; i >= 1; i--) {
        objects[i].type = OBJ_SHIELD_BALL;
        objects[i].timer = timerVal;
        timerVal -= 3;
    }

    Weapon_SubtractMagic();
    Sound_Play(SFX_SHIELD_BALL);
}

static void Weapon_InitSmasher(void) {
    if (objects[1].type) {
        return;
    }

    objects[1].type = OBJ_SMASHER;
    objects[1].timer = 3;
    objects[1].hp = 9;
    Weapon_SubtractMagic();
}

static void Weapon_InitFlash(void) {
    if (flashTimer) {
        return;
    }
    
    flashTimer = 24;
    Weapon_SubtractMagic();
    Sound_Play(SFX_ENEMY_KILL);
}

int Weapon_SetCollisionCoords(Sint16 x, Sint16 y) {
    int index = (currObjectIndex - 1) & 7;
    Uint8 lastCollided = weaponCoords[index].collided;

    weaponCoords[index].x = x;
    // sprites are 8x16 so this gets the center
    weaponCoords[index].y = y + 8;
    weaponCoords[index].spawned = 1;
    weaponCoords[index].collided = 0;

    return lastCollided;
}

void Weapon_EraseCollisionCoords(void) {
    // set weapon pos off the map
    Weapon_SetCollisionCoords(0, roomHeightMetatiles * METATILE_SIZE);
    int index = (currObjectIndex - 1) & 7;
    weaponCoords[index].spawned = 0;
}
