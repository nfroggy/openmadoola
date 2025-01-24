/* sound.c: Sound engine and output code
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

#include <stdio.h>
#include <string.h>

#include "alloc.h"
#include "blargg_apu.h"
#include "buffer.h"
#include "constants.h"
#include "db.h"
#include "game.h"
#include "mml.h"
#include "platform.h"
#include "rom.h"
#include "sound.h"
#include "util.h"

// audio settings
#define SOUND_FREQ 44100
#define SAMPLES_PER_FRAME (SOUND_FREQ / 60)
// how many frames of audio to store in the sound buffer (increase if your sound skips)
#define BUFFERED_FRAMES 3

static const char *soundFilenames[NUM_SOUNDS] = {
    [MUS_TITLE]       = "mml/mus_title.mml",
    [MUS_ENDING]      = "mml/mus_ending.mml",
    [MUS_START]       = "mml/mus_start.mml",
    [MUS_CLEAR]       = "mml/mus_clear.mml",
    [MUS_BOSS]        = "mml/mus_boss.mml",
    [MUS_ITEM]        = "mml/mus_item.mml",
    [MUS_GAME_OVER]   = "mml/mus_game_over.mml",
    [MUS_CAVE]        = "mml/mus_cave.mml",
    [MUS_FOREST]      = "mml/mus_forest.mml",
    [SFX_PERASKULL]   = "mml/sfx_peraskull.mml",
    [SFX_FIREBALL]    = "mml/sfx_fireball.mml",
    [MUS_CASTLE]      = "mml/mus_castle.mml",
    [SFX_SWORD]       = "mml/sfx_sword.mml",
    [SFX_MENU]        = "mml/sfx_menu.mml",
    [SFX_LUCIA_HIT]   = "mml/sfx_lucia_hit.mml",
    [SFX_BOMB]        = "mml/sfx_bomb.mml",
    [SFX_JUMP]        = "mml/sfx_jump.mml",
    [SFX_ENEMY_HIT]   = "mml/sfx_enemy_hit.mml",
    [SFX_BOMB_SPLIT]  = "mml/sfx_bomb_split.mml",
    [SFX_SHIELD_BALL] = "mml/sfx_shield_ball.mml",
    [SFX_NOMAJI]      = "mml/sfx_nomaji.mml",
    [SFX_BOUND_BALL]  = "mml/sfx_bound_ball.mml",
    [SFX_YOKKO_CHAN]  = "mml/sfx_yokko_chan.mml",
    [SFX_ENEMY_KILL]  = "mml/sfx_enemy_kill.mml",
    [SFX_ITEM]        = "mml/sfx_item.mml",
    [SFX_BOSS_KILL]   = "mml/sfx_boss_kill.mml",
    [SFX_PAUSE]       = "mml/sfx_pause.mml",
    [SFX_SELECT]      = "mml/sfx_select.mml",
    [SFX_FLAME_SWORD] = "mml/sfx_flame_sword.mml",
    [SFX_NYURU]       = "mml/sfx_nyuru.mml",
    [SFX_JOYRAIMA]    = "mml/sfx_joyraima.mml",
    [MUS_BOSS_ARCADE] = "mml/mus_boss_arcade.mml",
    [SFX_LUCIA_DEAD]  = "mml/sfx_lucia_dead.mml"
};

Sound sounds[NUM_SOUNDS];
// where sound data is stored in CHR ROM
#define CHR_ROM_SOUND (0x7B70)
// where sound data is stored in PRG ROM
#define PRG_ROM_SOUND (0x75DE)

// currently playing instruments
#define NUM_INSTRUMENTS 6
static Instrument instruments[NUM_INSTRUMENTS];
static Instrument savedInstruments[NUM_INSTRUMENTS];
static Instrument musInstruments[NUM_INSTRUMENTS];
static Instrument savedMusInstruments[NUM_INSTRUMENTS];
#define APU_CHANNELS 4
static Uint8 channelsInUse[APU_CHANNELS * 2];
static Uint8 apuStatusCopy[2];
// 0-100
static int volume = 50;
static int muted;

static void Sound_RunInstrument(int apu, Instrument *inst);
static void Sound_DisableChannel(int apu, Uint8 channel);
static void Sound_EnableChannel(int apu, Uint8 channel);

static Uint8 *Sound_ConvertData(Uint8 *instData) {
    Buffer *buf = Buffer_Init(16);
    int pos = 0;
    while (1) {
        // command: leave as is
        Uint8 cmd = *instData++;
        Buffer_Add(buf, cmd);
        // parameter: convert to Uint16
        Uint16 param = (Uint16)(*instData++);
        Buffer_AddUint16(buf, param);
        // if we're jumping backwards or we hit the end of the track, exit the loop
        if (((cmd == 0xbf) && (param < pos)) || (cmd == 0xff)) {
            break;
        }
        pos++;
    }
    // Buffer_Destroy also frees buff->data, which we don't want to do
    Uint8 *data = buf->data;
    free(buf);
    return data;
}

static Uint8 *Sound_LoadData(Uint8 *romData, Sound *out) {
    int cursor = 0;
    out->count = romData[cursor++];
    out->data = ommalloc(out->count * sizeof(Instrument));
    for (int i = 0; i < out->count; i++) {
        out->data[i].num = romData[cursor++];
        Uint16 addr = romData[cursor] | (romData[cursor + 1] << 8);
        cursor += 2;
        // title screen and ending music are stored in CHR ROM and loaded to
        // RAM during playback, so if we're seeing an address in RAM we know
        // that sound is stored in CHR ROM
        Uint8 *instData;
        if (addr < 0x8000) {
            // sound data is loaded to 0x400 in memory
            instData = chrRom + CHR_ROM_SOUND + addr - 0x400;
        }
        else {
            // PRG ROM is mapped into NES memory at 0x8000-0xFFFF
            instData = prgRom + (addr - 0x8000);
        }
        out->data[i].data = Sound_ConvertData(instData);
        out->data[i].channel = romData[cursor++];
        out->data[i].cursor = romData[cursor++];
        out->data[i].reg1 = romData[cursor++];
        out->data[i].reg0 = romData[cursor++];
        out->data[i].lastNote = 0;
    }

    return romData + cursor;
}

void Sound_Init(void) {
    Blargg_Apu_Init(SOUND_FREQ);
    DBEntry *entry = DB_Find("volume");
    if (entry) {
        volume = (int) entry->data[0];
    }
    Blargg_Apu_Volume(volume);
    muted = 0;
}

int Sound_LoadGameSounds(void) {
    // load sound data from the ROM
    Uint8 *src = chrRom + CHR_ROM_SOUND;
    for (int i = 0; i < NUM_ROM_SOUNDS; i++) {
        // override the sound with MML file if one is available
        if (MML_Compile(soundFilenames[i], &sounds[i])) {
            // skip past the sound definition in the ROM
            int count = (int)(*src++);
            src += (count * 7);
        }
        else {
            src = Sound_LoadData(src, &sounds[i]);
            // make sure you change this if you mess with the music order!
            sounds[i].isMusic = ((i < SFX_PERASKULL) || (i == MUS_CASTLE));
            // patch sound data to fix instrument allocation problems (why certain
            // sound effects would cause issues with other sound effects, music
            // channels dropping out, etc in the original game)
            switch (i) {
            case SFX_FIREBALL:
                sounds[i].data[0].num = 5;
                break;

            case SFX_PAUSE:
                sounds[i].data[0].num = 4;
                break;

            case SFX_SELECT:
                sounds[i].data[0].num = 5;
                sounds[i].data[1].num = 4;
                break;
            }
        }
        // sound data stored in CHR ROM is padded for some reason
        if (i == 0) { src = chrRom + CHR_ROM_SOUND + 0x20; }
        // after loading title screen and ending music, switch to PRG ROM
        if (i == 1) { src = prgRom + PRG_ROM_SOUND; }
    }
    for (int i = NUM_ROM_SOUNDS; i < NUM_SOUNDS; i++) {
        // these sounds aren't in the ROM, so if the MML is missing or broken we have to abort
        if (!MML_Compile(soundFilenames[i], &sounds[i])) {
            Platform_ShowError("Error compiling %s", soundFilenames[i]);
            return 0;
        }
    }
    return 1;
}

int Sound_SetVolume(int vol) {
    volume = vol;
    CLAMP(volume, 0, 100);
    Blargg_Apu_Volume(volume);
    Uint8 volumeByte = (Uint8)volume;
    DB_Set("volume", &volumeByte, 1);
    DB_Save();
    return volume;
}

int Sound_GetVolume(void) {
    return volume;
}

void Sound_Mute(void) {
    muted = 1;
}

void Sound_Unmute(void) {
    muted = 0;
}

char *Sound_GetDebugText(int num) {
    static char output[256] = {0};
    char row[64];
    Instrument *insts;
    if ((gameType != GAME_TYPE_ORIGINAL) && sounds[num].isMusic) {
        insts = musInstruments;
    }
    else {
        insts = instruments;
    }
    strcpy(output, "I C R0 R1 CURS NT TIMR LP\n\n");
    for (int i = 0; i < NUM_INSTRUMENTS; i++) {
        snprintf(row, sizeof(row), "%d %d %02x %02x %04x %02x %04x %02x\n\n",
                 i,
                 insts[i].channel,
                 insts[i].reg0,
                 insts[i].reg1,
                 insts[i].cursor,
                 insts[i].lastNote,
                 insts[i].timer,
                 insts[i].loop);
        strcat(output, row);
    }

    return output;
}

void Sound_Reset(void) {
    // initialize sound engine state
    for (int i = 0; i < NUM_INSTRUMENTS; i++) {
        instruments[i].cursor = 0xffff;
        musInstruments[i].cursor = 0xffff;
    }
    apuStatusCopy[0] = 0;
    apuStatusCopy[1] = 0;
    Blargg_Apu_Write(0, 0x4015, apuStatusCopy[0]);
    Blargg_Apu_Write(1, 0x4015, apuStatusCopy[1]);
    Blargg_Apu_ClearBuffer();
}

void Sound_Play(int num) {
    // copy all instruments from a sound into their respective slots
    Instrument *destInsts;
    int apu;
    if ((gameType != GAME_TYPE_ORIGINAL) && sounds[num].isMusic) {
        destInsts = musInstruments;
        apu = 1;
    }
    else {
        destInsts = instruments;
        apu = 0;
    }
    for (int i = 0; i < sounds[num].count; i++) {
        Uint8 instNum = sounds[num].data[i].num;
        // turn off the channel for the previous instrument in this slot
        // Note: The original game didn't do this, which is why sound effects
        // would sometimes stay on, etc.
        if (destInsts[instNum].channel < 4) {
            Sound_DisableChannel(apu, destInsts[instNum].channel);
        }
        // load sound data into the slot
        destInsts[instNum] = sounds[num].data[i];
        destInsts[instNum].timer = 1;
        destInsts[instNum].loop = 0xff;
        destInsts[instNum].ctrlRegsSet = 0xff;
    }
}

void Sound_SaveState(void) {
    memcpy(savedInstruments, instruments, sizeof(instruments));
    memcpy(savedMusInstruments, musInstruments, sizeof(musInstruments));
}

void Sound_LoadState(void) {
    memcpy(instruments, savedInstruments, sizeof(instruments));
    memcpy(musInstruments, savedMusInstruments, sizeof(musInstruments));
    // make sure each instrument sets its APU registers
    for (int i = 0; i < NUM_INSTRUMENTS; i++) {
        instruments[i].ctrlRegsSet = 0xff;
        musInstruments[i].ctrlRegsSet = 0xff;
    }
}

static void Sound_RunEngine(void) {
    memset(channelsInUse, 0, sizeof(channelsInUse));
    for (int i = NUM_INSTRUMENTS - 1; i >= 0; i--) {
        Sound_RunInstrument(0, &instruments[i]);
        Sound_RunInstrument(1, &musInstruments[i]);
    }
}

void Sound_Run(void) {
    static Sint16 buff0[SAMPLES_PER_FRAME * BUFFERED_FRAMES];
    static Sint16 buff1[SAMPLES_PER_FRAME * BUFFERED_FRAMES];

    // find the number of samples we need to fill up the audio buffer
    Sint32 neededSamples = (SAMPLES_PER_FRAME * BUFFERED_FRAMES) - Platform_GetQueuedSamples();

    for (Sint32 i = 0; i < neededSamples; i += SAMPLES_PER_FRAME) {
        Sound_RunEngine();
        Blargg_Apu_EndFrame();
    }
    Blargg_Apu_Samples(0, buff0, SAMPLES_PER_FRAME * BUFFERED_FRAMES);
    Sint32 outputSamples = Blargg_Apu_Samples(1, buff1, SAMPLES_PER_FRAME * BUFFERED_FRAMES);
    if (muted) {
        memset(buff0, 0, sizeof(buff0));
    }
    else {
        // mix output from both APUs together
        for (int i = 0; i < outputSamples; i++) {
            buff0[i] += buff1[i];
        }
    }
    Platform_QueueSamples(buff0, outputSamples);
}

static Uint16 freqTbl[] = {
    0xd5c,
    0xc9c,
    0xbe8,
    0xb3c,
    0xa9a,
    0xa02,
    0x972,
    0x8ea,
    0x86a,
    0x7f2,
    0x780,
    0x714,
};

static void Sound_RunInstrument(int apu, Instrument *inst) {
    Uint8 reg2, reg3;

    // "instrument not playing" flag
    if (inst->cursor == 0xffff) {
        return;
    }
    inst->timer--;
    if (inst->timer) {
        // "instrument still playing a note" flag
        reg2 = 0x6F;
    }
    else {
        setReadPtr:;
        Uint8 *read = inst->data + (inst->cursor * 3);
        getCmd:
        if (*read == 0xff) {
            inst->cursor = 0xffff;
            inst->loop = 0xff;
            channelsInUse[(apu * APU_CHANNELS) + inst->channel] = 0;
            Sound_DisableChannel(apu, inst->channel);
            goto lostChannel;
        }

        Uint8 cmd = *read++;
        Uint16 param = Util_LoadUint16(read);
        read += 2;

        // a0-af: APU register setting
        if ((cmd >= 0xa0) && (cmd < 0xb0)) {
            if (cmd & 1) {
                inst->reg1 = (Uint8)param;
            }
            else {
                inst->reg0 = (Uint8)param;
            }
            inst->cursor++;
            inst->ctrlRegsSet = 0xff;
            goto getCmd;
        }
        // b0-bf: looping/jumping
        else if ((cmd >= 0xb0) && (cmd < 0xc0)) {
            if (cmd == 0xbf) {
                inst->cursor = param;
                goto setReadPtr;
            }
            // loop over: set to 0xff (no loop) and move on
            if (!inst->loop) {
                inst->loop--;
                inst->cursor++;
                goto getCmd;
            }
            // new loop
            if (inst->loop == 0xff) {
                inst->loop = cmd & 0xf;
            }
            // loop in progress
            else {
                inst->loop--;
            }
            inst->cursor = param;
            goto setReadPtr;
        }
        // >= c0 or < a0: play note
        else {
            inst->cursor++;

            // noise channel: set registers directly
            if (inst->channel == 3) {
                reg2 = cmd;
                reg3 = 0;
            }
            // key off
            else if ((cmd & 0xf) >= 0xc) {
                Sound_DisableChannel(apu, inst->channel);
                reg2 = 0x6f;
                reg3 = 0;
            }
            else {
                Uint8 note = cmd & 0xf;
                Uint8 octave = cmd >> 4;
                Uint16 freq = freqTbl[note];
                freq >>= (octave + 1);
                reg2 = freq & 0xff;
                reg3 = freq >> 8;
            }

            inst->lastNote = cmd;
            inst->timer = param;
            // length counter load
            reg3 |= 8;
        }
    }
    // set up apu regs
    if (channelsInUse[(apu * APU_CHANNELS) + inst->channel]) {
        goto lostChannel;
    }
    channelsInUse[(apu * APU_CHANNELS) + inst->channel] = 0xff;
    if (reg2 == 0x6f) {
        return;
    }
    Sound_EnableChannel(apu, inst->channel);
    Uint32 regOffset = inst->channel * 4;
    if (inst->ctrlRegsSet) {
        Blargg_Apu_Write(apu, 0x4001 + regOffset, inst->reg1);
        Blargg_Apu_Write(apu, 0x4000 + regOffset, inst->reg0);
    }
    Blargg_Apu_Write(apu, 0x4002 + regOffset, reg2);
    Blargg_Apu_Write(apu, 0x4003 + regOffset, reg3);
    inst->ctrlRegsSet = 0;
    return;

    lostChannel:
    // when an instrument loses access to its channel, it has to reset its
    // registers when it regains access
    inst->ctrlRegsSet = 0xff;
}

static void Sound_DisableChannel(int apu, Uint8 channel) {
    apuStatusCopy[apu] &= ~(1 << channel);
    Blargg_Apu_Write(apu, 0x4015, apuStatusCopy[apu]);
}

static void Sound_EnableChannel(int apu, Uint8 channel) {
    apuStatusCopy[apu] |= (1 << channel);
    Blargg_Apu_Write(apu, 0x4015, apuStatusCopy[apu]);
}