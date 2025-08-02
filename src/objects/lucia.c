/* lucia.c: Lucia object code
 * Copyright (c) 2023, 2024, 2025 Nathan Misner
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

#include "camera.h"
#include "constants.h"
#include "game.h"
#include "item.h"
#include "joy.h"
#include "map.h"
#include "object.h"
#include "rng.h"
#include "sound.h"
#include "sprite.h"
#include "weapon.h"

#define WING_MP (1000)

Uint8 bootsLevel;
Uint8 attackTimer;
Uint8 hasWing;
Uint8 usingWing;
Uint8 luciaDoorFlag = 0;
Uint8 luciaHurtPoints;

Sint16 health;
Sint16 maxHealth;
Sint8 healthTimer;
Sint16 magic;
Sint16 maxMagic;
Sint8 lives;

Fixed16 luciaXPos;
Fixed16 luciaYPos;
Sint16 luciaSpriteX;
Sint16 luciaSpriteY;
Uint16 luciaMetatile;


static Sint8 xSpeeds[] = {
    0x00,0x00,0x18,0x18,0x18,0x00,0xE8,0xE8,0xE8,
    0x00,0x00,0x1C,0x1C,0x1C,0x00,0xE4,0xE4,0xE4,
    0x00,0x00,0x20,0x20,0x20,0x00,0xE0,0xE0,0xE0,
    0x00,0x00,0x28,0x28,0x28,0x00,0xD8,0xD8,0xD8,
};

static Sint8 ySpeeds[] = {
    0x00,0xF0,0xF0,0x00,0x10,0x10,0x10,0x00,0xF0,
    0x00,0xE8,0xE8,0x00,0x18,0x18,0x18,0x00,0xE8,
    0x00,0xE0,0xE0,0x00,0x20,0x20,0x20,0x00,0xE0,
    0x00,0xD8,0xD8,0x00,0x28,0x28,0x28,0x00,0xD8,
};

static Sint8 jumpSpeeds[] = {
    0xB4, 0xAE, 0xA0, 0x80,
};

static Uint8 luciaTiles[] = {
    0x06,0x04,0x0A,0x08,0x0E,0x0C,0x0A,0x08,
    0x02,0x00,0x06,0x04,0x2C,0x00,0x2A,0x28,
    0x22,0x20,0x22,0x20,0x2C,0x20,0x2A,0x20,
    0x26,0x24,0x26,0x24,0x2C,0x24,0x2A,0x24,
};

/**
 * @brief Runs code common to all Lucia objects and draws her sprites
 * @param o Lucia's object
 * @param frame The frame number to draw
*/
static void Lucia_Draw(Object *o, int frame);

void Lucia_NormalObj(Object *o) {
    o->xSpeed = xSpeeds[bootsLevel * 9 + joyDir];
    o->ySpeed = ySpeeds[bootsLevel * 9 + joyDir];
    // lucia can't move when she's attacking
    if (attackTimer) {
        o->xSpeed = 0;
    }
    Object_SetDirection(o);
    Object_UpdateXPos(o);
    
    // make lucia fall if she's walked off the ground
    if (!Object_TouchingGround(o)) {
        o->ySpeed = 0x20;
        o->type = OBJ_LUCIA_AIR;
        Lucia_Draw(o, 5);
        return;
    }

    if (joyEdge & JOY_A) {
        Sound_Play(SFX_JUMP);
        o->ySpeed = jumpSpeeds[bootsLevel];
        // if the boots level is at least 2, the player can control Lucia's
        // movement when she's jumping
        if (bootsLevel >= 2) {
            o->type = OBJ_LUCIA_AIR;
        }
        else {
            o->type = OBJ_LUCIA_AIR_LOCKED;
        }
        usingWing = 0;
        if (hasWing && (joy & JOY_DOWN)) {
            if ((magic - WING_MP) >= 0) {
                magic -= WING_MP;
                usingWing = 1;
            }
        }
        Lucia_Draw(o, 5);
        return;
    }

    // check to see if lucia should be climbing or not
    if ((o->xSpeed == 0) && o->ySpeed) {
        Uint16 collisionCheck = o->collision;
        // if moving down, check the below metatile
        if (o->ySpeed < 0) { collisionCheck -= roomWidthMetatiles; }
        // otherwise, check the above metatile
        else { collisionCheck += roomWidthMetatiles; }

        Uint16 currMetatile = mapMetatiles[collisionCheck];
        // lucia shouldn't climb on solid metatiles or scenery
        if ((currMetatile < MAP_SOLID) || (currMetatile >= MAP_LADDER)) {
            o->ySpeed = 0;
        }
        // the only possibility left is that lucia's on a ladder metatile
        else {
            // snap lucia to the ladder's metatile
            Object_MetatileAlignX(o);
            Object_UpdateYPos(o);
            o->type = OBJ_LUCIA_CLIMB;
            Lucia_Draw(o, 7);
            return;
        }

        // if it turns out Lucia shouldn't be climbing, set her y speed to 0
        o->ySpeed = 0;
    }

    // still or attacking
    if (attackTimer || (o->xSpeed == 0)) {
        // ducking
        if (joy & JOY_DOWN) {
            Lucia_Draw(o, 6);
            return;
        }
        // standing
        else {
            Lucia_Draw(o, 4);
            return;
        }
    }

    // running
    else {
        Lucia_Draw(o, (gameFrames >> 3) & 3);
        return;
    }
}

void Lucia_LvlEndDoorObj(Object *o) {
    // turn off Lucia's sprite when there's less than 35 frames left in the
    // animation to simulate her going through the door
    if (roomChangeTimer >= 35) {
        Lucia_Draw(o, 4);
    }
}

void Lucia_WarpDoorObj(Object *o) {
    // flash Lucia's sprite on and off every 4 frames
    if (roomChangeTimer & 4) {
        Lucia_Draw(o, 4);
    }
}

void Lucia_DyingObj(Object *o) {
    // first part of death animation: reuse warp door animation
    if (roomChangeTimer >= 90) {
        Lucia_WarpDoorObj(o);
    }
    // next part: have lucia kneeling for a few frames
    else if (roomChangeTimer >= 84) {
        Lucia_Draw(o, 6);
    }
    // last part: have lucia lying on the ground
    else {
        Sprite spr = { 0 };
        spr.size = SPRITE_16X16;
        spr.tile = 0x3E0;
        Sprite_SetDraw(&spr, o, 0, 0);
        if (o->direction == DIR_LEFT) {
            spr.x -= 16;
        }
        else {
            spr.x += 16;
        }

        spr.tile = 0x3E2;
        Sprite_DrawDir(&spr, o);
    }
}

void Lucia_ClimbObj(Object *o) {
    Uint16 collision;

    // handle jumping off the ladder
    if (joyEdge & JOY_A) {
        o->type = OBJ_LUCIA_AIR;
        Lucia_Draw(o, 5);
        return;
    }

    o->xSpeed = xSpeeds[bootsLevel * 9 + joyDir];
    o->ySpeed = ySpeeds[bootsLevel * 9 + joyDir];
    Object_SetDirection(o);

    // handle walking off the ladder
    if (o->xSpeed && (o->y.f.l >= 0x80)) {
        if (Object_GetMetatile(o) >= MAP_LADDER) {
            goto make_normal;
        }
    }

    if (o->ySpeed >= 0) {
        if (Object_UpdateYPos(o)) {
            goto make_normal;
        }

        collision = o->collision;
        if (mapMetatiles[collision] < MAP_LADDER) {
            goto make_climb;
        }

        collision += roomWidthMetatiles;
        if (mapMetatiles[collision] < MAP_LADDER) {
            goto make_climb;
        }

        if (o->y.f.l < 0x80) {
            goto make_climb;
        }
        goto make_air;

    }

    else {
        Object_UpdateYPos(o);
        if (o->y.f.l >= 0x80) {
            goto make_climb;
        }

        collision = o->collision;
        if (mapMetatiles[collision] < MAP_LADDER) {
            goto make_climb;
        }

        collision -= roomWidthMetatiles;
        if (mapMetatiles[collision] < MAP_LADDER) {
            goto make_climb;
        }
    }

make_normal:
    o->y.f.l = 0x80;
    o->type = OBJ_LUCIA_NORMAL;
    Lucia_Draw(o, 4);
    return;

make_air:
    o->type = OBJ_LUCIA_AIR;
    Lucia_Draw(o, 5);
    return;

make_climb:
    Lucia_Draw(o, 7);
    return;

}

void Lucia_AirObj(Object *o) {
    // this function gets used for both Lucia's air and air locked movement
    // objects, so this is the code that's unique to the air object
    if (o->type == OBJ_LUCIA_AIR) {
        o->xSpeed = xSpeeds[bootsLevel * 9 + joyDir];
        if (usingWing) {
            o->ySpeed = 0xe0;
            if (!(joy & JOY_A)) {
                usingWing = 0;
            }
        }
    }

    Object_SetDirection(o);
    Object_UpdateXPos(o);
    // falling, so check for ground
    if (o->ySpeed >= 0) {
        if (Object_TouchingGround(o)) {
            Object_MetatileAlignY(o);
            goto make_normal;
        }
    }

    if (Object_UpdateYPos(o)) {
        // make lucia stop rising when she hits her head on something
        o->ySpeed = 0;
        usingWing = 0;
    }

    // lucia's jump should be floatier when you're holding A
    if (!(joy & JOY_A)) {
        o->ySpeed += 0xc;
    }
    o->ySpeed += 7;

    // cap fall speed
    if (o->ySpeed > 0x40) {
        o->ySpeed = 0x40;
    }

    Lucia_Draw(o, 5);
    return;

make_normal:
    o->timer = 10;
    o->type = OBJ_LUCIA_NORMAL;
    Lucia_Draw(o, 4);
    return;
}

static Uint8 Lucia_GetAttackTile(int tileNo) {
    if (attackTimer < 6) {
        return tileNo + 16;
    }
    else {
        return tileNo + 8;
    }
}

/**
 * @brief increases Lucia's hit points or magic points
 * @param val XXXXXCTT
 * C: Point count.
 *    0 = increase by 500
 *    1 = increase by 100
 * 
 * TT: Point type
 *     0 = current HP
 *     1 = max HP
 *     2 = current MP
 *     3 = max MP
*/
static void Lucia_AddHPMP(Uint8 val) {
    int increase = (val & 0x4) ? 100 : 500;

    switch (val & 0x3) {
    case 0:
        health += increase;
        break;
    case 1:
        maxHealth += increase;
        break;
    case 2:
        magic += increase;
        break;
    case 3:
        maxMagic += increase;
        break;
    }

    // make sure the new values fall within their limits
    if (maxHealth > 5000) {
        maxHealth = 5000;
    }
    if (maxMagic > 5000) {
        maxMagic = 5000;
    }
    if (health > maxHealth) {
        health = maxHealth;
    }
    if (magic > maxMagic) {
        magic = maxMagic;
    }
}

// TODO refactor
static void Lucia_Draw(Object *o, int frame) {
    Sprite spr = { 0 };
    spr.size = SPRITE_16X16;
    spr.palette = 0;
    int tileNum = frame * 2;

    // skip past collision stuff if we've already hit a doorway
    if (roomChangeTimer) {
        attackTimer = 0;
        goto updateCamera;
    }

    luciaMetatile = Object_GetMetatile(o);
    if (!bossActive && !luciaDoorFlag && (luciaMetatile >= 0x9e) && (luciaMetatile < 0xa4)) {
        luciaDoorFlag = 0xff;

        // at warp door
        if (luciaMetatile >= 0xA0) {
            roomChangeTimer = 30;
            o->type = OBJ_LUCIA_WARP_DOOR;
        }
        // at end of level door
        else if ((stage < highestReachedStage) || orbCollected) {
            Sound_Reset();
            Sound_Play(MUS_CLEAR);
            roomChangeTimer = 210;
            o->type = OBJ_LUCIA_LVL_END_DOOR;
        }
    }
    else if (luciaDoorFlag && ((luciaMetatile < 0x9e) || (luciaMetatile >= 0xa4))) {
        luciaDoorFlag = 0;
    }

    // arcade mode drains 1 hp every 32 frames in normal or crazy difficulty when not standing at a door
    if ((gameType == GAME_TYPE_ARCADE) &&
        ((arcadeDifficulty == ARCADE_DIFF_NORMAL) || (arcadeDifficulty == ARCADE_DIFF_CRAZY)) &&
        (o->type != OBJ_LUCIA_WARP_DOOR) && (o->type != OBJ_LUCIA_LVL_END_DOOR) &&
        (++healthTimer >= 32))
    {
        healthTimer = 0;
        health--;
    }

    if ((health <= 0) && (o->type != OBJ_LUCIA_LVL_END_DOOR)) {
        health = 0;
        o->stunnedTimer = 0;
        roomChangeTimer = 150;
        o->type = OBJ_LUCIA_DYING;
        Sound_Reset();
        if (gameType == GAME_TYPE_ARCADE) {
            Sound_Play(SFX_LUCIA_DEAD);
        }
        else {
            Sound_Play(MUS_GAME_OVER);
            Sound_Play(SFX_LUCIA_HIT);
        }
    }

    if (roomChangeTimer) {
        o->x.f.l = 0x80;
        o->y.f.l = 0x80;
        scrollMode = SCROLL_MODE_LOCKED;
        Object_DeleteAllAfterLucia();
        goto updatePos;
    }

    if (luciaHurtPoints) {
        // 0xff is yokko-chan's collision value when not in arcade mode
        if ((luciaHurtPoints == 0xff) && !keywordDisplay) {
            // display the keyword if we haven't already
            keywordDisplay++;
        }
        // lucia collected an item
        else if (luciaHurtPoints >= ITEM_FLAG) {
            Sound_Play(SFX_ITEM);
            Uint8 itemType = luciaHurtPoints - ITEM_FLAG;
            // 0 - 6: power up the corresponding weapon
            if (itemType < 7) {
                weaponLevels[itemType]++;
                // clamp weapon level
                if (weaponLevels[itemType] >= 4) {
                    weaponLevels[itemType] = 3;
                }
            }
            else if (itemType == ITEM_BOOTS) {
                bootsLevel++;
                if (bootsLevel >= 4) {
                    bootsLevel = 3;
                }
            }
            else if (itemType == ITEM_ORB) {
                orbCollected = 0xff;
                // orb restores 500 hp
                Lucia_AddHPMP(0);
            }
            else {
                Lucia_AddHPMP(itemType);
            }
        }
        // enemy hit lucia
        else {
            // if lucia's already stunned, don't hit her again
            if (o->stunnedTimer == 0) {
                Sound_Play(SFX_LUCIA_HIT);
                // otherwise, stun her for 1 second
                o->stunnedTimer = 60;
                if (o->ySpeed) {
                    // if lucia's in the air, bump her down
                    o->ySpeed = 0x40;
                }
                else {
                    // otherwise, bump her up
                    // NOTE (bug from the original game): Because a Y speed of 0x80 is equivalent
                    // to boots level 3, this lets Lucia access areas she normally couldn't if the
                    // player is holding A when Lucia is hit. I decided not to fix this oversight
                    // because it makes movement more interesting and helps the player get out of
                    // a softlock on stage 4 (if playing correctly).
                    // The best way to fix the bug would probably be to make a new object type for
                    // Lucia being knocked back that points to the Lucia_AirObj function, and then
                    // make the "press A to jump higher" code in Lucia_AirObj dependent on the
                    // object type not being "Lucia knocked back".
                    o->ySpeed = 0x80;
                }
                // set a random x speed (-4 to 4 pixels)
                o->xSpeed = (rngVal & 0x7f) - 0x40;
                o->type = OBJ_LUCIA_AIR_LOCKED;
                health -= (luciaHurtPoints * 10);
                if (health < 0) {
                    health = 0;
                }
            }
        }
    }
    
    luciaHurtPoints = 0;
    if (o->stunnedTimer) {
        o->stunnedTimer--;
    }

updateCamera:
    // update camera from lucia's position
    Camera_LuciaScroll(o);

updatePos:
    luciaXPos = o->x;
    luciaYPos = o->y;

    // draw the level using the new scroll position
    Map_SetPos(cameraX.v >> 4, cameraY.v >> 4);
    // decrement attack timer
    if (attackTimer) { attackTimer--; }

    // set up direction
    Uint8 directionBak = o->direction;
    if (o->type == OBJ_LUCIA_CLIMB) {
        // this does the climbing animation
        o->direction = (o->y.f.h & 1) ? DIR_LEFT : DIR_RIGHT;
    }

    // draw lower body sprite
    if (attackTimer) { spr.tile = luciaTiles[Lucia_GetAttackTile(tileNum)]; }
    else { spr.tile = luciaTiles[tileNum]; }
    Uint8 yOffset = frame == 6 ? 8 : 0;
    Sprite_SetDraw(&spr, o, 0, yOffset);
    luciaSpriteX = spr.x;
    luciaSpriteY = spr.y;

    // draw upper body sprite
    spr.y -= 16;
    if ((o->type == OBJ_LUCIA_CLIMB) && attackTimer) {
        o->direction = directionBak; // make it so lucia faces the correct direction when she attacks
        spr.tile = attackTimer < 6 ? 0x24 : 0x20;
    }
    else {
        if (attackTimer) { spr.tile = luciaTiles[Lucia_GetAttackTile(tileNum) + 1]; }
        else { spr.tile = luciaTiles[tileNum + 1]; }
    }
    Sprite_DrawDir(&spr, o);
    o->direction = directionBak;
}
