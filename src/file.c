/* file.c: File management utility functions
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

// first because it contains the OM_UNIX define
#include "constants.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef OM_UNIX
#include <sys/stat.h>
#endif
#include "alloc.h"
#include "file.h"

#ifdef OM_UNIX
#define OM_XDG_DIR "openmadoola"
#define OM_HOME_DIR ".openmadoola"
static int filenameBuffLen = 256;
static char *filenameBuff = NULL;
#endif

void File_WriteUint16BE(Uint16 data, FILE *fp) {
    fputc(data >> 8, fp);
    fputc(data & 0xff, fp);
}

Uint16 File_ReadUint16BE(FILE *fp) {
    Uint16 i = fgetc(fp) << 8;
    i |= fgetc(fp);
    return i;
}

void File_WriteUint32BE(Uint32 data, FILE *fp) {
    fputc((data >> 24) & 0xff, fp);
    fputc((data >> 16) & 0xff, fp);
    fputc((data >>  8) & 0xff, fp);
    fputc(data & 0xff, fp);
}

Uint32 File_ReadUint32BE(FILE *fp) {
    Uint32 i = (Uint32)fgetc(fp) << 24;
    i |= ((Uint32)fgetc(fp) << 16);
    i |= ((Uint32)fgetc(fp) << 8);
    i |= (Uint32)fgetc(fp);
    return i;
}

#ifdef OM_UNIX
static void checkBuffSize(int size) {
    if (!filenameBuff) { filenameBuff = ommalloc(filenameBuffLen); }
    if (size > filenameBuffLen) {
        filenameBuffLen = size;
        filenameBuff = omrealloc(filenameBuff, filenameBuffLen);
    }
}

static FILE *getFileFromEnvvarDir(const char *envvar, const char *dir, const char *filename, const char *mode) {
    char *envvarPath = getenv(envvar);
    if (envvarPath) {
        checkBuffSize(strlen(envvarPath) + 1 + strlen(dir) + 1 + strlen(filename) + 1);
        int length = sprintf(filenameBuff, "%s/%s", envvarPath, dir);
        // try to make the directory in case it doesn't exist
        mkdir(filenameBuff, S_IRWXU);
        sprintf(filenameBuff + length, "/%s", filename);
        return fopen(filenameBuff, mode);
    }
    return NULL;
}
#endif

FILE *File_Open(const char *filename, const char *mode) {
#ifdef OM_UNIX
    FILE *fp;

    // first, try XDG data dir
    if ((fp = getFileFromEnvvarDir("XDG_DATA_HOME", OM_XDG_DIR, filename, mode))) {
        return fp;
    }
    // if that didn't work, try home dir
    if ((fp = getFileFromEnvvarDir("HOME", OM_HOME_DIR, filename, mode))) {
        return fp;
    }
    // if that didn't work, try current working dir
    return fopen(filename, mode);
#else
    return fopen(filename, mode);
#endif
}

#ifdef OM_UNIX
static char *resourceDirs[] = {
    NULL, // xdg data dir placeholder
    NULL, // ~/.openmadoola/ placeholder
    "/app/share/openmadoola/", // for flatpak
    "/usr/local/share/openmadoola/",
    "/usr/share/openmadoola/",
    "", // current working directory
};
#endif

FILE *File_OpenResource(const char *filename, const char *mode) {
    // set up home data directory name
#ifdef OM_UNIX
    if (!resourceDirs[0]) {
        char *xdgDataDir = getenv("XDG_DATA_HOME");
        if (xdgDataDir) {
            resourceDirs[1] = ommalloc(strlen(xdgDataDir) + 1 + strlen(OM_XDG_DIR) + 2);
            sprintf(resourceDirs[1], "%s/" OM_XDG_DIR "/", xdgDataDir);
        }
    }

    if (!resourceDirs[1]) {
        char *homedir = getenv("HOME");
        if (homedir) {
            resourceDirs[1] = ommalloc(strlen(homedir) + 1 + strlen(OM_HOME_DIR) + 2);
            sprintf(resourceDirs[1], "%s/" OM_HOME_DIR "/", homedir);
        }
    }

    for (int i = 0; i < ARRAY_LEN(resourceDirs); i++) {
        if (resourceDirs[i]) {
            checkBuffSize(strlen(resourceDirs[i]) + strlen(filename) + 1);
            strcpy(filenameBuff, resourceDirs[i]);
            strcat(filenameBuff, filename);
            FILE *fp = fopen(filenameBuff, mode);
            if (fp) { return fp; }
        }
    }
    return NULL;
#else
    return fopen(filename, mode);
#endif
}

Uint8 *File_Load(FILE *fp, int *size) {
    fseek(fp, 0, SEEK_END);
    int fileSize = ftell(fp);
    rewind(fp);
    Uint8 *data = ommalloc(fileSize);
    fread(data, 1, fileSize, fp);
    if (size) { *size = fileSize; }
    return data;
}