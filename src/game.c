/* game.c: Game related management code and global variables
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

#include <assert.h>
#include <string.h>

#include "camera.h"
#include "darutos.h"
#include "db.h"
#include "demo.h"
#include "ending.h"
#include "enemy.h"
#include "game.h"
#include "highscore.h"
#include "hud.h"
#include "item.h"
#include "joy.h"
#include "lucia.h"
#include "map.h"
#include "mainmenu.h"
#include "object.h"
#include "options.h"
#include "palette.h"
#include "pausemenu.h"
#include "platform.h"
#include "rng.h"
#include "rom.h"
#include "save.h"
#include "screen.h"
#include "sound.h"
#include "sprite.h"
#include "system.h"
#include "task.h"
#include "title.h"
#include "weapon.h"

#define SOFT_RESET (JOY_A | JOY_B | JOY_START | JOY_SELECT)

Uint8 gameType = GAME_TYPE_PLUS;
Uint8 arcadeDifficulty = ARCADE_DIFF_NORMAL;
Uint8 paused;
Uint8 stage;
Uint8 highestReachedStage;
Uint8 orbCollected;
Uint8 roomChangeTimer;
Uint8 bossActive;
Uint8 numBossObjs;
Uint8 bossDefeated[16];
Uint8 gameFrames;
Sint8 keywordDisplay;
Uint8 fountainUsed;
// arcade stuff
Uint32 score;

Uint8 spritePalettes[16] = {
    0x00, 0x12, 0x16, 0x36,
    0x00, 0x1A, 0x14, 0x30,
    0x00, 0x01, 0x11, 0x26,
    0x00, 0x00, 0x27, 0x37,
};

static void Game_InitRoomVars(Object *lucia);
static void Game_SpawnFountain(SpawnInfo *info);
static void Game_InitNewGame(void);
static void Game_InitCommon(void);
static void Game_InitDemo(DemoData *data);
static void Game_Run(void);
static int Game_RunStage(void);
static void Game_DrawHud(void);
static void Game_HandleWeaponSwitch(void);
static void Game_HandlePause(void);
static void Game_SetRoom(Uint8 roomNum);
static void Game_HandlePaletteShifting(void);
static void Game_HandleRoomChange(void);

typedef enum {
    STAGE_EXIT_NEXTSTAGE,
    STAGE_EXIT_DIED,
    STAGE_EXIT_WON,
    STAGE_EXIT_DEMO_RECORDING_QUIT,
} GameRunStageExit;
static int Game_RunStage(void);

void Game_LoadSettings(void) {
    // initialize game type
    DBEntry *entry = DB_Find("gametype");
    gameType = entry ? entry->data[0] : GAME_TYPE_PLUS;
    entry = DB_Find("arcadediff");
    arcadeDifficulty = entry ? entry->data[0] : ARCADE_DIFF_NORMAL;
}

void Game_NewGame(void) {
    Game_InitNewGame();
    Save_SaveFile();
    Game_InitCommon();
    RNG_Seed();
    Game_Run();
}

void Game_LoadGame(void) {
    health = 1000;
    Game_InitCommon();
    RNG_Seed();
    Game_Run();
}

static void Game_InitNewGame(void) {
    health = 1000;
    maxHealth = 1000;
    maxMagic = 1000;
    highestReachedStage = 0;

    // initialize boots and weapon levels
    bootsLevel = 0;
    memset(weaponLevels, 0, NUM_WEAPONS);
    weaponLevels[WEAPON_SWORD] = 1;
    Item_InitCollected();
    memset(bossDefeated, 0, sizeof(bossDefeated));
    stage = 0;
    orbCollected = 0;
    keywordDisplay = 0;
}

static void Game_InitCommon(void) {
    if (mapData) {
        Map_FreeData(mapData);
    }
    if (gameType == GAME_TYPE_ARCADE) {
        mapData = Rom_GetMapDataArcade();
    }
    else {
        mapData = Rom_GetMapData();
    }
    score = 0;
    lives = 3;
    paused = 0;
    currentWeapon = WEAPON_SWORD;
}

static Uint8 itemSpawnYOffsets[] = {0x09, 0x0B, 0x09, 0x0B};
static void Game_InitRoomVars(Object *lucia) {
    Map_LoadPalettes(currRoom);
    // load sprite palettes
    Sprite_SetAllPalettes(spritePalettes);
    // clear all weapon objects
    Weapon_Init();

    // set up the camera
    cameraX.f.h = lucia->x.f.h & 0x70;
    cameraY.f.h = (lucia->y.f.h & 0x70) + 1;
    cameraX.f.l = 0;
    cameraY.f.l = 0;

    bossActive = 0;
    luciaHurtPoints = 0;
    roomChangeTimer = 0;
    attackTimer = 0;
    lucia->direction = 0;
    flashTimer = 0;

    SpawnInfo info;
    Map_GetSpawnInfo(lucia, &info);

    // if we're in an item room and the item hasn't been collected, spawn it
    if ((info.type == SPAWN_TYPE_ITEM) && (!Item_Collected(lucia))) {
        objects[9].type = OBJ_ITEM;
        objects[9].hp = info.enemy - ITEM_FLAG;
        objects[9].x.f.h = (lucia->x.f.h & 0x70) | 7;
        objects[9].y.f.h = (lucia->y.f.h & 0x70) | itemSpawnYOffsets[lucia->y.f.h >> 5];
        objects[9].x.f.l = 0x80;
        objects[9].y.f.l = 0x80;
        objects[9].ySpeed = 0;
    }
    
    // if we're in the boss room and the boss hasn't been defeated, set up the
    // number of boss objects
    else if (currRoom == 6) {
        if (!bossDefeated[stage]) {
            bossActive = 1;
            numBossObjs = mapData->stages[stage].bossObjCount;
        }
    }

    // Arcade mode only allows using a fountain once per life per stage.
    // Otherwise, Lucia can stay in a stage indefinitely and rack up a really
    // high score which breaks the scoring system.
    else if ((info.type == SPAWN_TYPE_FOUNTAIN) &&
             ((gameType != GAME_TYPE_ARCADE) || !fountainUsed))
    {
        Game_SpawnFountain(&info);
    }

    // spawn the wing of madoola if lucia hasn't collected it yet
    if (stage == 15) {
        if (!hasWing) {
            objects[MAX_OBJECTS - 1].type = OBJ_WING_OF_MADOOLA;
        }
        // NOTE: This wasn't in the original game. This fixes a bug where
        // collecting the Wing of Madoola and then going into a door would
        // cause Darutos not to spawn, softlocking the game.
        else {
            objects[MAX_OBJECTS - 1].type = OBJ_DARUTOS_INIT;
        }
    }

    lucia->type = OBJ_LUCIA_NORMAL;
    Camera_SetXY(lucia);
    Object_InitCollision(lucia);
}

static Sint8 fountainXTbl[] = {0x39, 0x29, 0x59, 0x79, 0x29, 0x59, 0x79};
static Sint8 fountainYTbl[] = {0x0A, 0x1A, 0x1A, 0x1A, 0x26, 0x26, 0x26};
static Uint8 fountainPalette[] = {0x26, 0x03, 0x31, 0x21};
static void Game_SpawnFountain(SpawnInfo *info) {
    Uint8 offset = (info->enemy & 0x7) - 1;
    objects[9].x.f.h = fountainXTbl[offset];
    objects[9].y.f.h = fountainYTbl[offset];
    objects[9].type = OBJ_FOUNTAIN;
    Sprite_SetPalette(2, fountainPalette);
}

static void Game_Run(void) {
    while (1) {
        switch (Game_RunStage()) {
        case STAGE_EXIT_NEXTSTAGE:
            stage++;
            stage &= 0xf;
            if (stage > highestReachedStage) {
                highestReachedStage = stage;
                orbCollected = 0;
            }
            Save_SaveFile();
            break;

        case STAGE_EXIT_DIED:
            if (gameType == GAME_TYPE_ARCADE) {
                lives--;
                if (lives < 1) {
                    Save_SaveFile();
                    Screen_GameOver();
                    HighScore_NameEntry(score);
                    Task_Switch(Title_Run);
                }
                else {
                    health = 1000;
                }
            }
            else {
                Save_SaveFile();
                Screen_GameOver();
                Task_Switch(Save_Screen);
            }
            break;

        case STAGE_EXIT_WON:
            Save_SaveFile();
            Ending_Run();
            if (gameType == GAME_TYPE_ARCADE) {
                HighScore_NameEntry(score);
            }
            Task_Switch(Title_Run);
            break;

        case STAGE_EXIT_DEMO_RECORDING_QUIT:
            Task_Switch(Title_Run);
            break;
        }
    }
}

static Uint8 recordDemoInitialized = 0;
void Game_RecordDemoInit(char *filename, Uint8 _gameType, Uint8 _stage, Sint16 _health, Sint16 _magic, Uint8 _bootsLevel, Uint8 *_weaponLevels) {
    DemoData data;
    data.rngVal = rngVal;
    data.gameFrames = gameFrames;
    data.gameType = _gameType;
    data.stage = _stage;
    data.health = _health;
    data.magic = _magic;
    data.bootsLevel = _bootsLevel;
    memcpy(data.weaponLevels, _weaponLevels, sizeof(data.weaponLevels));
    Game_InitDemo(&data);
    Demo_Record(filename, &data);
    recordDemoInitialized = 1;
}

void Game_RecordDemoTask(void) {
    assert(recordDemoInitialized);
    Game_RunStage();
    Demo_Save();
    recordDemoInitialized = 0;
    Platform_Quit();
}

void Game_PlayDemo(char *filename) {
    DemoData data;
    if (!Demo_Playback(filename, &data)) {
        Platform_ShowError("Couldn't open demo file %s.", filename);
        return;
    };
    Game_InitDemo(&data);
    rngVal = data.rngVal;
    gameFrames = data.gameFrames;
    Game_RunStage();
    Demo_Uninit();
}

static void Game_InitDemo(DemoData *data) {
    Game_InitNewGame();
    Game_InitCommon();
    gameType = data->gameType;
    stage = data->stage;
    health = data->health;
    maxHealth = data->health;
    magic = data->magic;
    maxMagic = data->magic;
    bootsLevel = data->bootsLevel;
    memcpy(weaponLevels, data->weaponLevels, sizeof(weaponLevels));
}

static int Game_RunStage(void) {
    // the last room number Lucia was in this stage
    Uint16 lastRoom = 0xffff;

    Object_ListInit();
    // in arcade mode, health refills up to 1000 between stages
    if (gameType == GAME_TYPE_ARCADE) {
        health = MAX(health, 1000);
    }
    // on normal/hard difficulty, magic only refills up to 1000 between stages
    if ((gameType == GAME_TYPE_ARCADE) &&
        ((arcadeDifficulty == ARCADE_DIFF_NORMAL) || (arcadeDifficulty == ARCADE_DIFF_HARD)))
    {
        magic = MAX(magic, 1000);
    }
    else {
        magic = maxMagic;
    }

    Sound_Reset();
    Sound_Play(MUS_START);
    if (gameType == GAME_TYPE_ARCADE) {
        Screen_Stage();
        Screen_Status();
    }
    else {
        Screen_Status();
        Screen_Stage();
    }

    // set up lucia's position and the room number
    Object *lucia = &objects[0];
    memset(lucia, 0, sizeof(Object));
    lucia->x = mapData->stages[stage].xPos;
    lucia->y = mapData->stages[stage].yPos;
    Game_SetRoom(mapData->stages[stage].roomNum);
    healthTimer = 0;
    fountainUsed = 0;
    hasWing = 0;
    darutosKilled = 0;

initRoom:
    Game_InitRoomVars(lucia);
    if ((gameType == GAME_TYPE_ORIGINAL) || (currRoom != lastRoom)) {
        Game_PlayRoomSong();
    }
    lastRoom = currRoom;

    while (1) {
        gameFrames++;
        Game_HandlePaletteShifting();
        Task_Yield();
        Sprite_ClearOverlayList();
        // when recording a demo, pressing start ends the demo recording
        if (Demo_Recording() && (joyEdge & JOY_START)) {
            return STAGE_EXIT_DEMO_RECORDING_QUIT;
        }
        Game_HandlePause();
        Game_DrawHud();
        if (!paused) {
            Sprite_ClearList();
            RNG_Get(); // update RNG once per frame
            Weapon_Process();
            Object_ListRun();
            Enemy_Spawn();
            Game_HandleRoomChange();
        }
        Map_Draw();
        // if we're paused or an odd number of frames, draw the hud over the game sprites
        if (paused || (gameFrames & 1)) {
            Sprite_Display();
            Sprite_DisplayOverlay();
        }
        else { // if on an even frame, draw the game sprites over the hud (lets you see enemies that are under the hud)
            Sprite_DisplayOverlay();
            Sprite_Display();
        }

        // --- handle keyword screen ---
        if (keywordDisplay > 0) {
            Screen_Keyword();
            // mark keyword as shown
            keywordDisplay = -1;
            // despawn yokko-chan
            Object_DeleteRange(9);
            // force music to play
            lastRoom = 0xffff;
            goto initRoom;
        }

        // --- handle doors ---
        if (roomChangeTimer == 1) {
            if (lucia->type == OBJ_LUCIA_WARP_DOOR) {
                int switchRoom = Map_Door(lucia);
                if (switchRoom == DOOR_ENDING) {
                    if (darutosKilled) {
                        return STAGE_EXIT_WON;
                    }
                    // put Lucia back where she was if she tried to enter the ending
                    // door without killing Darutos
                    else {
                        Game_SetRoom(currRoom);
                    }
                }
                else if (switchRoom == DOOR_INVALID) {
                    Game_SetRoom(currRoom);
                }
                else {
                    Game_SetRoom(switchRoom);
                }
                goto initRoom;
            }
            else if (lucia->type == OBJ_LUCIA_LVL_END_DOOR) {
                return STAGE_EXIT_NEXTSTAGE;
            }
            else {
                return STAGE_EXIT_DIED;
            }
        }
    }
}

void Game_PlayRoomSong(void) {
    Sound_Reset();
    Uint8 song = mapData->rooms[currRoom].song;
    // are we in stage 16's room?
    if (currRoom == 14) {
        // don't play any music if darutos has been killed
        if (darutosKilled) {
            return;
        }
        // if lucia collected the wing of madoola, play the castle theme
        if (hasWing) {
            Sound_Play(song);
        }
        // otherwise play the boss room theme
        else {
            if (gameType == GAME_TYPE_ORIGINAL) {
                Sound_Play(MUS_BOSS);
            }
            else {
                Sound_Play(MUS_BOSS_ARCADE);
            }
        }
    }
    // if we're in the boss room, play the boss music if the boss hasn't been
    // killed, and the item room music if it has been
    else if (song == MUS_BOSS) {
        if (bossActive) {
            if (gameType == GAME_TYPE_ORIGINAL) {
                Sound_Play(MUS_BOSS);
            }
            else {
                Sound_Play(MUS_BOSS_ARCADE);
            }
        }
        else {
            Sound_Play(MUS_ITEM);
        }
    }
    else {
        Sound_Play(song);
    }
}

void Game_AddScore(Uint32 points) {
    Uint32 oldScore = score;
    score += points;
    // on arcade normal/hard modes, Lucia's MP refils when she scores a multiple of 10000 points
    if ((gameType == GAME_TYPE_ARCADE) &&
        ((arcadeDifficulty == ARCADE_DIFF_NORMAL) || (arcadeDifficulty == ARCADE_DIFF_HARD)) &&
        ((score / 10000) > (oldScore / 10000)))
    {
        magic = maxMagic;
    }
    // keep score inside 8 numbers
    if (score >= 100000000) {
        score -= 100000000;
    }
}

static void Game_DrawHud(void) {
    switch (gameType) {
    case GAME_TYPE_ORIGINAL:
        HUD_DisplayOriginal(health, magic);
        if (paused) {
            Game_HandleWeaponSwitch();
            HUD_Weapon((SCREEN_WIDTH / 2) - (16 / 2), 32);
        }
        break;

    case GAME_TYPE_PLUS:
        Game_HandleWeaponSwitch();
        HUD_DisplayPlus(health, magic);
        break;

    case GAME_TYPE_ARCADE:
        Game_HandleWeaponSwitch();
        HUD_DisplayArcade(health, magic, score);
        break;
    }
}

static void Game_HandleWeaponSwitch(void) {
    if (joyEdge & JOY_SELECT) {
        Sound_Play(SFX_SELECT);
        Weapon_Init();
        // go to the next weapon that we have and have enough magic to use
        do {
            currentWeapon++;
            if (currentWeapon >= NUM_WEAPONS) { currentWeapon = 0; }
        } while ((weaponLevels[currentWeapon] == 0) || (Weapon_MagicAfterUse() < 0));
        // clamp weapon level to 3
        if (weaponLevels[currentWeapon] > 3) {
            weaponLevels[currentWeapon] = 3;
        }
    }
}

static void Game_HandlePause(void) {
    if (paused) {
        switch (PauseMenu_Run()) {
        case PAUSE_EXIT_RESUME:
            if (gameType == GAME_TYPE_ORIGINAL) {
                Game_PlayRoomSong();
            }
            else {
                Sound_LoadState();
            }
            Sound_Play(SFX_PAUSE);
            paused = 0;
            break;

        case PAUSE_EXIT_QUIT:
            Task_Switch(Title_Run);
            break;

        default:
            break;
        }
    }
    else if (joyEdge & JOY_START) {
        Sound_SaveState();
        Sound_Reset();
        Sound_Play(SFX_PAUSE);
        paused = 1;
        PauseMenu_Init();
    }
}

static void Game_SetRoom(Uint8 roomNum) {
    if (roomNum == 15) { scrollMode = SCROLL_MODE_LOCKED; }
    else if ((roomNum == 6) || (roomNum == 7)) { scrollMode = SCROLL_MODE_X; }
    else { scrollMode = SCROLL_MODE_FREE; }
    Map_Init(roomNum);
}

static void Game_HandlePaletteShifting(void) {
    // only palette shift on rooms with waterfalls
    if ((currRoom != 1) && (currRoom != 2)) {
        return;
    }

    if ((gameFrames & 3) == 0) {
        Uint8 temp = colorPalette[15];
        colorPalette[15] = colorPalette[14];
        colorPalette[14] = colorPalette[13];
        colorPalette[13] = temp;
    }
}

static Uint16 *Game_GetDoorMetatiles(void) {
    static Uint16 metatiles[6];

    // get Lucia's collision offset
    Uint16 offset = objects[0].collision;
    Uint16 yOffset = offset / roomWidthMetatiles;
    Uint16 xOffset = offset % roomWidthMetatiles;

    // chunk align offset numbers
    xOffset &= 0xFC;
    yOffset &= 0xFC;

    // the exit door chunks have the door offset by 1 metatile from the top left of the chunk
    xOffset += 1;
    yOffset += 1;

    // get door's metatile offset
    offset = yOffset * roomWidthMetatiles + xOffset;

    // 3 left door metatiles
    metatiles[0] = mapMetatiles[offset];
    metatiles[1] = mapMetatiles[offset + roomWidthMetatiles];
    metatiles[2] = mapMetatiles[offset + roomWidthMetatiles * 2];

    // 3 right door metatiles
    metatiles[3] = mapMetatiles[offset + 1];
    metatiles[4] = mapMetatiles[offset + roomWidthMetatiles + 1];
    metatiles[5] = mapMetatiles[offset + roomWidthMetatiles * 2 + 1];

    return metatiles;
}

static void Game_SetMetatileTiles(Uint16 num, Uint16 tl, Uint16 tr, Uint16 bl, Uint16 br) {
    Uint16 tileset = mapData->rooms[currRoom].tileset;
    Uint16 base = tilesetBases[tileset];

    mapData->tilesets[tileset].metatiles[num].tiles[0] = tl + base;
    mapData->tilesets[tileset].metatiles[num].tiles[1] = tr + base;
    mapData->tilesets[tileset].metatiles[num].tiles[2] = bl + base;
    mapData->tilesets[tileset].metatiles[num].tiles[3] = br + base;
}

static void Game_LeftDoorMidOpen(void) {
    Uint16 *metatiles = Game_GetDoorMetatiles();
    Game_SetMetatileTiles(metatiles[0], 0xd6, 0xff, 0xe6, 0xff);
    Game_SetMetatileTiles(metatiles[1], 0xe8, 0xff, 0xe6, 0xff);
    Game_SetMetatileTiles(metatiles[2], 0xe8, 0xff, 0xf6, 0xff);
}

static void Game_LeftDoorFullOpen(void) {
    Uint16 *metatiles = Game_GetDoorMetatiles();
    Game_SetMetatileTiles(metatiles[0], 0xff, 0xff, 0xff, 0xff);
    Game_SetMetatileTiles(metatiles[1], 0xff, 0xff, 0xff, 0xff);
    Game_SetMetatileTiles(metatiles[2], 0xff, 0xff, 0xff, 0xff);
}

static void Game_LeftDoorClose(void) {
    Uint16 *metatiles = Game_GetDoorMetatiles();
    Game_SetMetatileTiles(metatiles[0], 0xd5, 0xd6, 0xe5, 0xe6);
    Game_SetMetatileTiles(metatiles[1], 0xe7, 0xe8, 0xe5, 0xe6);
    Game_SetMetatileTiles(metatiles[2], 0xe7, 0xe8, 0xf5, 0xf6);
}

static void Game_RightDoorMidOpen(void) {
    Uint16 *metatiles = Game_GetDoorMetatiles();
    Game_SetMetatileTiles(metatiles[3], 0xff, 0xd7, 0xff, 0xe7);
    Game_SetMetatileTiles(metatiles[4], 0xff, 0xe5, 0xff, 0xe7);
    Game_SetMetatileTiles(metatiles[5], 0xff, 0xe5, 0xff, 0xf5);
}

static void Game_RightDoorFullOpen(void) {
    Uint16 *metatiles = Game_GetDoorMetatiles();
    Game_SetMetatileTiles(metatiles[3], 0xff, 0xff, 0xff, 0xff);
    Game_SetMetatileTiles(metatiles[4], 0xff, 0xff, 0xff, 0xff);
    Game_SetMetatileTiles(metatiles[5], 0xff, 0xff, 0xff, 0xff);
}

static void Game_RightDoorClose(void) {
    Uint16 *metatiles = Game_GetDoorMetatiles();
    Game_SetMetatileTiles(metatiles[3], 0xd7, 0xd8, 0xe7, 0xe8);
    Game_SetMetatileTiles(metatiles[4], 0xe5, 0xe6, 0xe7, 0xe8);
    Game_SetMetatileTiles(metatiles[5], 0xe5, 0xe6, 0xf5, 0xf6);
}

static void Game_HandleRoomChange(void) {
    if (roomChangeTimer) {
        // if Lucia's at the end of level door, animate the door opening
        if (objects[0].type == OBJ_LUCIA_LVL_END_DOOR) {
            switch (roomChangeTimer) {
            case 255:
                return;

            case 60:
                Game_LeftDoorMidOpen();
                break;

            case 59:
                Game_RightDoorMidOpen();
                break;

            case 45:
                Game_LeftDoorFullOpen();
                break;

            case 44:
                Game_RightDoorFullOpen();
                break;

            case 30:
                Game_LeftDoorMidOpen();
                break;

            case 29:
                Game_RightDoorMidOpen();
                break;

            case 15:
                Game_LeftDoorClose();
                break;

            case 14:
                Game_RightDoorClose();
                break;
            }
        }

        roomChangeTimer--;
    }
}
