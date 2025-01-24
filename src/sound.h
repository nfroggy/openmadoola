/* sound.h: Sound engine and output code
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

#pragma once

typedef enum {
    MUS_TITLE       = 0,
    MUS_ENDING      = 1,
    MUS_START       = 2,
    MUS_CLEAR       = 3,
    MUS_BOSS        = 4,
    MUS_ITEM        = 5,
    MUS_GAME_OVER   = 6,
    MUS_CAVE        = 7,
    MUS_FOREST      = 8,
    SFX_PERASKULL   = 9,
    SFX_FIREBALL    = 10,
    MUS_CASTLE      = 11,
    SFX_SWORD       = 12,
    SFX_MENU        = 13,
    SFX_LUCIA_HIT   = 14,
    SFX_BOMB        = 15,
    SFX_JUMP        = 16,
    SFX_ENEMY_HIT   = 17,
    SFX_BOMB_SPLIT  = 18,
    SFX_SHIELD_BALL = 19,
    SFX_NOMAJI      = 20,
    SFX_BOUND_BALL  = 21,
    SFX_YOKKO_CHAN  = 22,
    SFX_ENEMY_KILL  = 23,
    SFX_ITEM        = 24,
    SFX_BOSS_KILL   = 25,
    SFX_PAUSE       = 26,
    SFX_SELECT      = 27,
    SFX_FLAME_SWORD = 28,
    SFX_NYURU       = 29,
    SFX_JOYRAIMA    = 30,
    NUM_ROM_SOUNDS  = 31,
    MUS_BOSS_ARCADE = 31,
    SFX_LUCIA_DEAD,
    NUM_SOUNDS,
} SOUND_NUM;

// state of a playing instrument
typedef struct {
    Uint8 num;
    Uint8 *data;
    Uint8 channel;
    Uint16 cursor;
    Uint8 reg0;
    Uint8 reg1;
    Uint16 timer;
    Uint8 loop;
    Uint8 ctrlRegsSet;
    Uint8 lastNote;
} Instrument;

typedef struct {
    Uint8 isMusic;
    Uint8 count;
    Instrument *data;
} Sound;

extern Sound sounds[NUM_SOUNDS];

/**
 * @brief Initializes sound output
*/
void Sound_Init(void);

/**
 * @brief Loads all the game sounds from either the ROM or MML files off disk.
 * It's necessary to run this function before starting the game.
 * @returns 1 on success, 0 on failure.
 */
int Sound_LoadGameSounds(void);

/**
 * @brief Sets the volume.
 * @param vol Percentage (0-100)
 * @returns the new volume value
 */
int Sound_SetVolume(int vol);

/**
 * @returns the current volume.
 */
int Sound_GetVolume(void);

/**
 * @brief Mutes the sound. Overrides any volume setting.
 */
void Sound_Mute(void);

/**
 * @brief Unmutes the sound. Does nothing unless sound is muted.
 */
void Sound_Unmute(void);

/**
 * @param num Sound number we want to inspect (picks the correct APU)
 * @returns informative text about the state of the sound engine (mainly used
 * for taking up space on the sound test screen)
 */
char *Sound_GetDebugText(int num);

/**
 * @brief Resets the sound engine (so nothing is playing)
*/
void Sound_Reset(void);

/**
 * @brief Plays a sound number
 * @param num the sound to play
*/
void Sound_Play(int num);

/**
 * @brief Saves the sound engine state
*/
void Sound_SaveState(void);

/**
 * @brief Loads the sound engine state
*/
void Sound_LoadState(void);

/**
 * @brief Runs the sound engine and outputs audio, should be called each frame
 * you want audio playing
*/
void Sound_Run(void);
