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
#include "config_db.h"
#include "constants.h"
#include "highscore.h"
#include "joy.h"
#include "sound.h"
#include "system.h"
#include "task.h"
#include "title.h"
#include "util.h"

#define NAME_SIZE 6
typedef struct {
    char name[NAME_SIZE];
    Uint32 score;
} HighScore;

// Kenji Sada (main programmer of The Wing of Madoola and sole programmer of the
// arcade port) put credits in the high score table if you read the names
// vertically.
// MOROTTAR: Naohisa Morota (sound)
// SIMOMURA: Character graphics
// SYUGIURA: Kazuyuki Sugiura (BG graphics & map design)
// ATSUSHII: Atsushi Sakai (sub-programmer)
// NAKAGAWA: Sub-programmer

// NOTE: Originally these scores were all 10x higher, which made it difficult
// to beat them. My POV is that it should be easy to beat the default high
// scores.
#define NUM_SCORES 8
static HighScore defaultScores[NUM_SCORES] = {
    {.name = "MSSAN", .score = 40000},
    {.name = "OIYTA", .score = 35000},
    {.name = "RMUSK", .score = 30000},
    {.name = "OOGUA", .score = 25000},
    {.name = "TMISG", .score = 20000},
    {.name = "TUUHA", .score = 15000},
    {.name = "ARRIW", .score = 10000},
    {.name = "RAAIA", .score =  5000},
};
static HighScore scores[NUM_SCORES];
static Uint32 lastScore;

static char *rankStrings[NUM_SCORES] = {
    "TOP", "2ND", "3RD", "4TH", "5TH", "6TH", "7TH", "8TH",
};

static char highScoreCharset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789. ";

static Uint8 highScorePalette[] = {
    0x0F, 0x26, 0x20, 0x20,
    0x0F, 0x36, 0x20, 0x20,
};

void HighScore_Init(void) {
    DBEntry *entry = DB_Find(&configDB, "highscores");
    if (entry) {
        int cursor = 0;
        for (int i = 0; i < NUM_SCORES; i++) {
            memcpy(&scores[i].name, entry->data + cursor, NAME_SIZE);
            cursor += NAME_SIZE;
            scores[i].score = Util_LoadUint32(entry->data + cursor);
            cursor += 4;
        }
    }
    else {
        HighScore_ResetScores();
    }
    lastScore = 0;
}

static void HighScore_Save(void) {
    Uint8 buffer[sizeof(scores)];
    int cursor = 0;
    for (int i = 0; i < NUM_SCORES; i++) {
        memcpy(buffer + cursor, &scores[i].name, NAME_SIZE);
        cursor += NAME_SIZE;
        Util_SaveUint32(scores[i].score, buffer + cursor);
        cursor += sizeof(Uint32);
    }
    DB_Set(&configDB, "highscores", buffer, cursor);
    DB_Save(&configDB);
}

void HighScore_ResetScores(void) {
    memcpy(&scores, &defaultScores, sizeof(scores));
    HighScore_Save();
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

Uint32 HighScore_GetLastScore(void) {
    return lastScore;
}

void HighScore_NameEntry(Uint32 score) {
    lastScore = score;
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
    memcpy(scores[scoreNum].name, "     ", NAME_SIZE);

    BG_Clear();
    BG_Scroll(BG_CENTERED_X, 0);
    // delay is to add space in between the game over music and the high score
    // music
    for (int i = 0; i < 60; i++) {
        BG_Display();
        Task_Yield();
    }
    BG_Print(6, 3, 1, "CONGRATULATIONS, YOU\n\nGOT A HIGH SCORE.");
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
        // start immediately ends the high score entry
        if (joyEdge & JOY_START) { exitFlag = 1; }
        // letter selection
        if (joyEdge & (JOY_LEFT | JOY_RIGHT)) {
            if (joyEdge & JOY_LEFT) { charCursor--; }
            if (joyEdge & JOY_RIGHT) { charCursor++; }
            Sound_Play(SFX_MENU);
            repeatTimer = 30;
        }
        else if (joy & (JOY_LEFT | JOY_RIGHT)) {
            repeatTimer--;
            if (repeatTimer <= 0) {
                if (joy & JOY_LEFT) { charCursor--; }
                if (joy & JOY_RIGHT) { charCursor++; }
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
                // set charCursor to the value of the letter we're on
                for (int i = 0; i < ARRAY_LEN(highScoreCharset); i++) {
                    if (highScoreCharset[i] == scores[scoreNum].name[nameCursor]) {
                        charCursor = i;
                        break;
                    }
                }
                Sound_Play(SFX_FLAME_SWORD);
            }
        }

        if (nameCursor < (NAME_SIZE - 1)) {
            BG_PutChar(nameX + nameCursor, nameY, 1, highScoreCharset[charCursor]);
        }

        BG_Display();
        Task_Yield();
    }

    // if player is content to remain anonymous
    if (!nameCursor) {
        memcpy(scores[scoreNum].name, "-----", NAME_SIZE);
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
        BG_Display();
        Task_Yield();
        if (joyEdge) { break; }
    }
    Sound_Reset();
}
