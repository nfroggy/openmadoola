/* rom.c: Reads from the game ROM file
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "constants.h"
#include "file.h"
#include "map.h"
#include "platform.h"
#include "rom.h"

Uint8 prgRom[PRG_ROM_SIZE];
Uint8 *chrRom = NULL;
int chrRomSize = 0;

Uint16 tilesetBases[3] = {
    1024, // Forest
    1280, // Cave
    1536, // Castle
};

static int Rom_LoadFromSteam(FILE *fp) {
    int size;
    Uint8 *steamData = File_Load(fp, &size);
    fclose(fp);

    // look for ROM in data file
    char searchStr[] = "MadoolaRAGdump";
    int searchStrLen = sizeof(searchStr);
    Uint8 *ptr = steamData;
    int found = 0;
    while (ptr) {
        int sizeLeft = size - (int)(ptr - steamData);
        ptr = memchr(ptr, searchStr[0], sizeLeft);
        if (ptr && (sizeLeft >= searchStrLen) && (memcmp(ptr, searchStr, searchStrLen) == 0)) {
            found = 1;
            break;
        }
        else if (ptr) {
            ptr++;
        }
    }
    if (!found) {
        Platform_ShowError("Couldn't find ROM file in Steam data");
        free(steamData);
        return 0;
    }

    // read in data
    ptr += 36;
    memcpy(prgRom, ptr, PRG_ROM_SIZE);
    ptr += PRG_ROM_SIZE;
    chrRomSize = 0x8000;
    chrRom = ommalloc(chrRomSize);
    memcpy(chrRom, ptr, chrRomSize);
    free(steamData);
    return 1;
}

static int Rom_LoadFromNesFile(FILE *fp) {
    int size = 0;
    Uint8 *romData = File_Load(fp, &size);
    fclose(fp);
    if (size != 65552) {
        free(romData);
        return 0;
    }
    memcpy(prgRom, romData + 0x10, PRG_ROM_SIZE);
    chrRomSize = 0x8000;
    chrRom = ommalloc(chrRomSize);
    memcpy(chrRom, romData + 0x10 + PRG_ROM_SIZE, chrRomSize);
    free(romData);
    return 1;
}

int Rom_Load(void) {
    FILE *fp;

    // try to load data from the rom file
    fp = File_OpenResource("madoola.nes", "rb");
    if (fp && Rom_LoadFromNesFile(fp)) {
        return 1;
    }

    // if there's no rom file, look for the asset file from the Sunsoft collection
    fp = File_OpenResource("sharedassets0.assets", "rb");
    if (fp && Rom_LoadFromSteam(fp)) {
        return 1;
    }

    // if there's no asset file, try loading from the default install location for the Sunsoft collection
#ifdef OM_WINDOWS
    fp = _wfopen(L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\SUNSOFT is Back! レトロゲームセレクション\\SUNSOFT is Back! Retro Game Selection_Data\\sharedassets0.assets", L"rb");
    if (fp && Rom_LoadFromSteam(fp)) {
        return 1;
    }
#endif

    Platform_ShowError("Couldn't find madoola.nes or sharedassets0.assets. Check the readme file for more information.");
    return 0;
}

int Rom_LoadChr(char *filename, int size) {
    FILE *fp = File_OpenResource(filename, "rb");
    if (!fp) {
        Platform_ShowError("Rom_LoadChr: Couldn't find %s", filename);
        return 0;
    }

    chrRom = omrealloc(chrRom, chrRomSize + size);
    fread(chrRom + chrRomSize, 1, size, fp);
    fclose(fp);

    chrRomSize += size;
    return 1;
}
