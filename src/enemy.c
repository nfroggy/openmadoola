/* enemy.c: Enemy spawning code
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

#include "daltos.h"
#include "game.h"
#include "camera.h"
#include "lucia.h"
#include "map.h"
#include "object.h"
#include "rng.h"

#define ENEMY_SLOT (9)

static int Enemy_InitLocation(Object *o) {
    o->type = OBJ_NONE;
    o->x.v = cameraX.v + 0x80;
    // spawn on right side of screen
    if (RNG_Get() < 0x80) {
        o->direction = DIR_LEFT;
        o->x.f.h += (SCREEN_WIDTH / 16) - 1;
    }
    // spawn on left side of screen
    else {
        o->direction = DIR_RIGHT;
    }
    o->stunnedTimer = 0;
    // "randomly" pick a y value
    o->y.f.l = 0x80;
    o->y.f.h = (gameFrames & 0xf) + cameraY.f.h;
    Object_InitCollision(o);
    // don't allow spawning enemies off the bottom of the map
    if (o->collision >= (roomWidthMetatiles * roomHeightMetatiles)) {
        return 0;
    }
    else {
        return 1;
    }
}

void Enemy_Spawn(void) {
    Uint16 rand = RNG_GetWithCarry();
    // simulate rotating right 4 times (6502 cycles carry bit through rotation)
    // before: [C]76543210
    // after:  [3]210C7654
    Uint16 low5 = (rand & 0x1f0) >> 4;
    Uint16 high4 = (rand & 0xf) << 5;
    rand = low5 | high4;
    // frame count gets added without clearing the carry flag
    rand += gameFrames;
    if (rand & 0x100) { rand++; }
    rand &= 0xff;

    // SCROLL_MODE_LOCKED happens when we're transitioning to a different
    // room, Lucia dies, or we're in an item room
    if ((scrollMode != SCROLL_MODE_LOCKED) && (rand < 0x20)) {
        // don't spawn enemies if lucia is in the last stage and has collected
        // the wing of madoola
        if (hasWing && (currRoom == 14)) {
            if (gameType != GAME_TYPE_ARCADE) { return; }
            // arcade mode starts spawning enemies again after daltos is killed
            else if (!daltosKilled) { return; }
        }

        Object temp = { 0 };
        if (!Enemy_InitLocation(&temp)) { return; }
        SpawnInfo info = { 0 };

        Map_GetSpawnInfo(&temp, &info);
        // return if the map data isn't valid
        if ((info.count == 0) || (info.count >= 10)) {
            return;
        }
        // return if we're spawning a boss and the boss has already been killed
        if ((info.type == SPAWN_TYPE_BOSS) && !bossActive) {
            return;
        }

        // limit spawning objects for boss areas
        if (bossActive) {
            info.enemy = mapData->stages[stage].bossObj;
            info.count = MIN(numBossObjs, mapData->stages[stage].bossSpawnCount);
        }

        // get the object slot
        Object *o = Object_FindNext(ENEMY_SLOT, ENEMY_SLOT + info.count);
        // if no available slot, return
        if (!o) {
            return;
        }
        *o = temp;

        // normal randomly spawning enemy
        if (info.type == SPAWN_TYPE_ENEMY) {
            o->type = info.enemy;
        }

        // boss
        else if (info.type == SPAWN_TYPE_BOSS) {
            if (!bossActive) {
                return;
            }
            o->type = info.enemy;
        }

        Object_FaceLucia(o);
        o->timer = 0;
    }
}
