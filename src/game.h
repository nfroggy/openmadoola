/* game.h: Game related management code and global variables
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

#pragma once
#include "constants.h"

#define GAME_TYPE_ORIGINAL 0
#define GAME_TYPE_PLUS 1
#define GAME_TYPE_ARCADE 2
extern Uint8 gameType;

#define ARCADE_DIFF_NORMAL 0
#define ARCADE_DIFF_HARD 1
#define ARCADE_DIFF_CRAZY 2
extern Uint8 arcadeDifficulty;

extern Uint8 spritePalettes[16];
extern Uint8 stage;
extern Uint8 highestReachedStage;
extern Uint8 orbCollected;
extern Uint8 roomChangeTimer;
extern Uint8 bossActive;
extern Uint8 numBossObjs;
extern Sint8 keywordDisplay;
extern Uint8 bossDefeated[16];
extern Uint8 gameFrames;
extern Uint8 fountainUsed;

/**
 * @brief Loads game-specific settings from disk.
 */
void Game_LoadSettings(void);

/**
 * @brief Starts a new game.
 */
void Game_NewGame(void);

/**
 * @brief Runs a game without initializing the state. Should only be run after loading a save file.
 */
void Game_LoadGame(void);

/**
 * @brief Gets ready to record a demo.
 * @param filename what to save the demo as
 * @param _gameType game type number
 * @param _stage stage number
 * @param _health health/max health
 * @param _magic magic/max magic
 * @param _bootsLevel boots level
 * @param _weaponLevels weapon levels (must have size NUM_WEAPONS)
 */
void Game_RecordDemoInit(char *filename, Uint8 _gameType, Uint8 _stage, Sint16 _health, Sint16 _magic, Uint8 _bootsLevel, Uint8 *_weaponLevels);

/**
 * @brief Plays a stage and records the demo. Should only be run (as a task) after Game_RecordDemoInit.
 */
void Game_RecordDemoTask(void);

/**
 * @brief Plays back a stage demo.
 * @param filename demo file to load
 */
void Game_PlayDemo(char *filename);

/**
 * @brief Plays the song associated with the current room.
*/
void Game_PlayRoomSong(void);

/**
 * @brief Adds points to the score, and caps it to 99999999.
 * @param points number of points to add
 */
void Game_AddScore(Uint32 points);