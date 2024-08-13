/* highscore.c: High score code
 * Copyright (c) 2024 Nathan Misner
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

#include "bg.h"
#include "buffer.h"
#include "constants.h"
#include "db.h"
#include "game.h"
#include "joy.h"
#include "sound.h"
#include "system.h"

#define NAME_SIZE 6
typedef struct {
    char name[NAME_SIZE];
    Uint32 score;
} HighScore;

#define NUM_SCORES 8
static char *rankStrings[NUM_SCORES] = {
    "TOP", "2ND", "3RD", "4TH", "5TH", "6TH", "7TH", "8TH",
};

static HighScore scores[NUM_SCORES];
static HighScore defaultScores[NUM_SCORES] = {
    {.name = "MSSAN", .score = 400000},
    {.name = "OIYTA", .score = 350000},
    {.name = "RMUSK", .score = 300000},
    {.name = "OOGUA", .score = 250000},
    {.name = "TMISG", .score = 200000},
    {.name = "TUUHA", .score = 150000},
    {.name = "ARRIW", .score = 100000},
    {.name = "RAAIA", .score =  50000},
};

static Uint8 highScorePalette[] = {
    0x0F, 0x26, 0x20, 0x20,
    0x0F, 0x36, 0x20, 0x20,
};

static char highScoreCharset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789. ";

void HighScore_Init(void) {
    DBEntry *entry = DB_Find("highscores");
    if (entry) {
        int cursor = 0;
        for (int i = 0; i < NUM_SCORES; i++) {
            memcpy(&scores[i].name, entry->data + cursor, NAME_SIZE);
            cursor += NAME_SIZE;
            scores[i].score = Buffer_DataReadUint32(entry->data + cursor);
            cursor += 4;
        }
    }
    else {
        memcpy(&scores, &defaultScores, sizeof(scores));
    }
}

static void HighScore_Save(void) {
    Uint8 buffer[sizeof(scores)];
    int cursor = 0;
    for (int i = 0; i < NUM_SCORES; i++) {
        memcpy(buffer + cursor, &scores[i].name, NAME_SIZE);
        cursor += NAME_SIZE;
        buffer[cursor++] = (scores[i].score >> 24) & 0xff;
        buffer[cursor++] = (scores[i].score >> 16) & 0xff;
        buffer[cursor++] = (scores[i].score >>  8) & 0xff;
        buffer[cursor++] = scores[i].score & 0xff;
    }
    DB_Set("highscores", buffer, cursor);
    DB_Save();
}

void HighScore_Print(int x, int y) {
    BG_SetPalette(0, highScorePalette);
    BG_SetPalette(1, highScorePalette + 4);
    BG_Print(x, y, 0, "RANK  NAME    SCORE");
    for (int i = 0; i < NUM_SCORES; i++) {
        BG_Print(x, (y + 3) + (i * 2), 0, "%s   %s   %08d", rankStrings[i], scores[i].name, scores[i].score);
    }
}

Uint32 HighScore_GetTopScore(void) {
    return scores[0].score;
}

void HighScore_NameEntry(void) {
    int scoreNum;
    for (scoreNum = 0; scoreNum < NUM_SCORES; scoreNum++) {
        if (score > scores[scoreNum].score) {
            // move all the other scores down 1
            for (int i = NUM_SCORES - 1; i > scoreNum; i--) {
                scores[i] = scores[i - 1];
            }
            break;
        }
    }
    // return if we didn't place in the high score list
    if (scoreNum >= NUM_SCORES) {
        return;
    }

    // initialize name & score
    scores[scoreNum].score = score;
    strcpy(scores[scoreNum].name, "     ");

    BG_Clear();
    BG_Print(6, 3, 0, "CONGRATULATIONS, YOU\n\nGOT A HIGH SCORE.");
    int scoreTableY = 8;
    HighScore_Print(6, scoreTableY);
    Sound_Reset();
    Sound_Play(MUS_TITLE);

    int nameX = 12;
    int nameY = (scoreTableY + 3) + (scoreNum * 2);
    int nameCursor = 0;
    int charCursor = 0;
    int repeatTimer = -1;
    int frames = 0;
    int exitFlag = 0;
    while (!exitFlag) {
        frames++;
        System_StartFrame();
        // start immediately ends the high score entry
        if (joyEdge & JOY_START) { exitFlag = 1; }
        // letter selection
        if (joyEdge & JOY_LEFT) {
            charCursor--;
            Sound_Play(SFX_MENU);
            repeatTimer = 30;
        }
        if (joyEdge & JOY_RIGHT) {
            charCursor++;
            Sound_Play(SFX_MENU);
            repeatTimer = 30;
        }
        if (joy & JOY_LEFT) {
            repeatTimer--;
            if (repeatTimer <= 0) {
                charCursor--;
                Sound_Play(SFX_MENU);
                repeatTimer = 5;
            }
        }
        if (joy & JOY_RIGHT) {
            repeatTimer--;
            if (repeatTimer <= 0) {
                charCursor++;
                Sound_Play(SFX_MENU);
                repeatTimer = 5;
            }
        }
        // handle wraparound
        if (charCursor < 0) { charCursor = ARRAY_LEN(highScoreCharset) - 2; }
        if (charCursor >= (ARRAY_LEN(highScoreCharset) - 1)) { charCursor = 0; }
        if (joyEdge & JOY_A) {
            scores[scoreNum].name[nameCursor] = highScoreCharset[charCursor];
            BG_PutChar(nameX + nameCursor, nameY, 0, highScoreCharset[charCursor]);
            nameCursor++;
            Sound_Play(SFX_SELECT);
            if (nameCursor >= (NAME_SIZE - 1)) {
                exitFlag = 1;
            }
        }
        else if (joyEdge & JOY_B) {
            if (nameCursor) {
                BG_PutChar(nameX + nameCursor, nameY, 0, ' ');
                scores[scoreNum].name[nameCursor--] = ' ';
                Sound_Play(SFX_FLAME_SWORD);
            }
        }

        if (nameCursor < (NAME_SIZE - 1)) {
            BG_PutChar(nameX + nameCursor, nameY, 1, highScoreCharset[charCursor]);
        }

        BG_Draw();
        System_EndFrame();
    }

    // if player is content to remain anonymous
    if (!nameCursor) {
        strcpy(scores[scoreNum].name, "-----");
        BG_Print(nameX, nameY, 0, "%s", scores[scoreNum].name);
    }
    // otherwise, cover up the cursor tile
    else {
        BG_PutChar(nameX + nameCursor, nameY, 0, ' ');
    }
    HighScore_Save();

    // wait on the high score screen for a few seconds
    frames = 300;
    while ((frames-- > 0)) {
        System_StartFrame();
        BG_Draw();
        System_EndFrame();
        if (joyEdge) { break; }
    }
    Sound_Reset();
}
