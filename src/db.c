/* db.c: "database" handler (more like a crappy version of RIFF i guess)
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

// db file spec
// Uint32: num entries
// each entry:
// Uint8: name length
// name
// Uint32: data length
// data

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "constants.h"
#include "db.h"
#include "file.h"
#include "platform.h"

#define DB_FILENAME "config.db"

static int numEntries;
static int allocedEntries;
static DBEntry *entries;

Uint8 *DB_Add(const char *name, Uint32 dataLen) {
    if (numEntries >= allocedEntries) {
        allocedEntries *= 2;
        entries = omrealloc(entries, allocedEntries * sizeof(DBEntry));
    }

    entries[numEntries].name = ommalloc(strlen(name) + 1);
    strcpy(entries[numEntries].name, name);
    entries[numEntries].dataLen = dataLen;
    entries[numEntries].data = ommalloc(dataLen);
    return entries[numEntries++].data;
}

DBEntry *DB_Find(const char *name) {
    // probably should change this if we ever go above ~50 entries (aka never)
    for (int i = 0; i < numEntries; i++) {
        if (strcmp(entries[i].name, name) == 0) {
            return &entries[i];
        }
    }
    return NULL;
}

void DB_Set(const char *name, Uint8 *data, Uint32 dataLen) {
    DBEntry *entry = DB_Find(name);
    if (entry) {
        entry->dataLen = dataLen;
        if (entry->data) { free(entry->data); }
        entry->data = ommalloc(dataLen);
        memcpy(entry->data, data, dataLen);
    }
    else {
        Uint8 *dbData = DB_Add(name, dataLen);
        memcpy(dbData, data, dataLen);
    }
}

void DB_Init(void) {
    // init vars
    allocedEntries = 10;
    entries = ommalloc(allocedEntries * sizeof(DBEntry));
    numEntries = 0;

    // load db file from disk
    FILE *fp = File_Open(DB_FILENAME, "rb");
    if (fp) {
        // name can't be more than 256 bytes because of the length field
        char name[256];
        Uint32 dataBuffSize = 256;
        Uint8 *dataBuff = ommalloc(dataBuffSize);
        Uint32 savedEntries = File_ReadUint32BE(fp);
        for (Uint32 i = 0; i < savedEntries; i++) {
            Uint8 nameLen = fgetc(fp);
            fread(name, 1, nameLen, fp);
            Uint32 dataLen = File_ReadUint32BE(fp);
            if (dataLen > dataBuffSize) {
                dataBuffSize = dataLen;
                dataBuff = omrealloc(dataBuff, dataBuffSize);
            }
            fread(dataBuff, 1, dataLen, fp);
            DB_Set(name, dataBuff, dataLen);
        }
        free(dataBuff);
        fclose(fp);
    }
}

void DB_Save(void) {
    FILE *fp = File_Open(DB_FILENAME, "wb");
    if (!fp) {
        Platform_ShowError("Couldn't open " DB_FILENAME " for writing");
        return;
    }

    File_WriteUint32BE((Uint32)numEntries, fp);
    for (int i = 0; i < numEntries; i++) {
        // add 1 for the NUL terminator
        Uint8 nameLen = (Uint8)strlen(entries[i].name) + 1;
        fputc(nameLen, fp);
        fwrite(entries[i].name, 1, nameLen, fp);
        File_WriteUint32BE(entries[i].dataLen, fp);
        fwrite(entries[i].data, 1, entries[i].dataLen, fp);
    }
    fclose(fp);
}
