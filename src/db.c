/* db.c: "database" handler (more like a crappy version of RIFF i guess)
 * Copyright (c) 2023-2026 Nathan Misner
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

static Uint8 *DB_Add(DBState *db, const char *name, Uint32 dataLen) {
    if (db->numEntries >= db->allocedEntries) {
        db->allocedEntries *= 2;
        db->entries = omrealloc(db->entries, db->allocedEntries * sizeof(DBEntry));
    }

    db->entries[db->numEntries].name = ommalloc(strlen(name) + 1);
    strcpy(db->entries[db->numEntries].name, name);
    db->entries[db->numEntries].dataLen = dataLen;
    db->entries[db->numEntries].data = ommalloc(dataLen);
    return db->entries[db->numEntries++].data;
}

DBEntry *DB_Find(DBState *db, const char *name) {
    // probably should change this if we ever go above ~50 entries (aka never)
    for (int i = 0; i < db->numEntries; i++) {
        if (strcmp(db->entries[i].name, name) == 0) {
            return &db->entries[i];
        }
    }
    return NULL;
}

void DB_Set(DBState *db, const char *name, Uint8 *data, Uint32 dataLen) {
    DBEntry *entry = DB_Find(db, name);
    if (entry) {
        entry->dataLen = dataLen;
        if (entry->data) { free(entry->data); }
        entry->data = ommalloc(dataLen);
        memcpy(entry->data, data, dataLen);
    }
    else {
        Uint8 *dbData = DB_Add(db, name, dataLen);
        memcpy(dbData, data, dataLen);
    }
}

void DB_Init(DBState *db, const char *filename) {
    // init vars
    db->allocedEntries = 10;
    db->entries = ommalloc(db->allocedEntries * sizeof(DBEntry));
    db->numEntries = 0;
    char *filename_copy = ommalloc(strlen(filename) + 1);
    strcpy(filename_copy, filename);
    db->filename = filename_copy;

    // load db file from disk
    FILE *fp = File_Open(db->filename, "rb");
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
            DB_Set(db, name, dataBuff, dataLen);
        }
        free(dataBuff);
        fclose(fp);
    }
}

void DB_Save(DBState *db) {
    FILE *fp = File_Open(db->filename, "wb");
    if (!fp) {
        Platform_ShowError("Couldn't open %s for writing", db->filename);
        return;
    }

    File_WriteUint32BE((Uint32)db->numEntries, fp);
    for (int i = 0; i < db->numEntries; i++) {
        // add 1 for the NUL terminator
        Uint8 nameLen = (Uint8)strlen(db->entries[i].name) + 1;
        fputc(nameLen, fp);
        fwrite(db->entries[i].name, 1, nameLen, fp);
        File_WriteUint32BE(db->entries[i].dataLen, fp);
        fwrite(db->entries[i].data, 1, db->entries[i].dataLen, fp);
    }
    fclose(fp);
}
