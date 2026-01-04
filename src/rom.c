/* rom.c: Reads from the game ROM file
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

static int init_tileset(MapData *data, int num, int length, Uint16 base, int pal_offset, int rom_offset) {
    data->tilesets[num].len = length;
    data->tilesets[num].metatiles = ommalloc(length * sizeof(Metatile));
    for (int i = 0; i < length; i++) {
        data->tilesets[num].metatiles[i].palnum   = (Uint16)prgRom[pal_offset++];
        data->tilesets[num].metatiles[i].tiles[0] = (Uint16)prgRom[rom_offset++] + base;
        data->tilesets[num].metatiles[i].tiles[1] = (Uint16)prgRom[rom_offset++] + base;
        data->tilesets[num].metatiles[i].tiles[2] = (Uint16)prgRom[rom_offset++] + base;
        data->tilesets[num].metatiles[i].tiles[3] = (Uint16)prgRom[rom_offset++] + base;
    }

    return rom_offset;
}

MapData *Rom_GetMapData(void) {
    MapData *data = ommalloc(sizeof(MapData));
    // position in the PRG ROM
    int cursor = 0;

    // there are 3 tilesets (forest, cave, castle)
    data->numTilesets = 3;
    data->tilesets = ommalloc(data->numTilesets * sizeof(Tileset));

    // forest tileset has 164 metatiles
    #define FOREST_PALS (0x2790)
    cursor = init_tileset(data, 0, 164, tilesetBases[0], FOREST_PALS, cursor);

    // cave tileset has 164 metatiles
    #define CAVE_PALS (0x2834)
    cursor = init_tileset(data, 1, 164, tilesetBases[1], CAVE_PALS, cursor);

    // castle tileset has 164 metatiles (noticing a pattern?)
    #define CASTLE_PALS (0x28d8)
    cursor = init_tileset(data, 2, 164, tilesetBases[2], CASTLE_PALS, cursor);

    // there are 223 chunks
    data->numChunks = 223;
    data->chunks = ommalloc(data->numChunks * sizeof(data->chunks[0]));
    for (int i = 0; i < data->numChunks; i++) {
        for (int j = 0; j < ARRAY_LEN(data->chunks[0]); j++) {
            data->chunks[i][j] = (Uint16)prgRom[cursor++];
        }
    }

    // there are 159 screens
    data->numScreens = 159;
    data->screens = ommalloc(data->numScreens * sizeof(data->screens[0]));
    for (int i = 0; i < data->numScreens; i++) {
        for (int j = 0; j < ARRAY_LEN(data->screens[0]); j++) {
            data->screens[i][j] = (Uint16)prgRom[cursor++];
        }
    }

    // there are 16 rooms
    data->numRooms = 16;
    data->rooms = ommalloc(data->numRooms * sizeof(Room));
    for (int i = 0; i < 16; i++) {
        // get tileset
        #define ROOM_BANK_TBL (0x44bf)
        #define BASE_BANK 4
        data->rooms[i].tileset = (Uint16)(prgRom[ROOM_BANK_TBL + i] >> 4) - BASE_BANK;

        // get song
        #define ROOM_SONG_TBL (0x2b4f)
        data->rooms[i].song = prgRom[ROOM_SONG_TBL + i];
        // original game didn't have song index 8 defined, so this fixes that issue
        if (data->rooms[i].song >= 9) { data->rooms[i].song--; }

        #define ROOM_PALETTE_TBL (0x35ff)
        for (int j = 0; j < ARRAY_LEN(data->rooms[0].palette); j++) {
            int palette_offset = ROOM_PALETTE_TBL + (i * 16) + j;
            data->rooms[i].palette[j] = prgRom[palette_offset];
        }

        // dimensions
        data->rooms[i].width = 8;
        data->rooms[i].height = 8;
        data->rooms[i].screenNums = ommalloc(data->rooms[i].width * data->rooms[i].height * sizeof(Uint16));

        // copy room data
        int numScreens = data->rooms[i].width * data->rooms[i].height;
        for (int j = 0; j < numScreens; j++) {
            data->rooms[i].screenNums[j] = (Uint16)prgRom[cursor++];
        }
        // in the original game, enemies were stored as offsets from a base
        // value that depended on the room number
        int room_sprite_bank = prgRom[ROOM_BANK_TBL + i] & 0x3;
        #define ENEMY_BASE_TBL (0x5bc8)
        Uint8 room_obj_base = prgRom[ENEMY_BASE_TBL + room_sprite_bank];
        for (int j = 0; j < ARRAY_LEN(data->rooms[0].spawns); j++) {
            Uint8 enemy_val = prgRom[cursor++];
            if (enemy_val >= 0xA0) {
                // items are A0-B0
                if (enemy_val <= 0xB0) {
                    data->rooms[i].spawns[j].type = SPAWN_TYPE_ITEM;
                }
                // fountains are B1-B7
                else if ((enemy_val & 0xF0) == 0xB0) {
                    data->rooms[i].spawns[j].type = SPAWN_TYPE_FOUNTAIN;
                }
                // bosses are D0-D1
                else {
                    data->rooms[i].spawns[j].type = SPAWN_TYPE_BOSS;
                    // D1 = "spawn boss on this screen", D0 = "don't"
                    data->rooms[i].spawns[j].count = (enemy_val & 0xf);
                }
                data->rooms[i].spawns[j].enemy = enemy_val;
            }
            else {
                data->rooms[i].spawns[j].type = SPAWN_TYPE_ENEMY;
                // low nybble = enemy object number offset
                data->rooms[i].spawns[j].enemy = (enemy_val & 0xf) + room_obj_base;
                // high nybble = enemy count
                data->rooms[i].spawns[j].count = enemy_val >> 4;
            }
        }
    }

    // load stage init data
    #define STAGE_INIT_TBL (0x2af6)
    #define BOSS_TYPE_TBL (0x5bcc)
    #define BOSS_COUNT_TBL (0x443b)
    for (int i = 0; i < 16; i++) {
        data->stages[i].xPos.f.h = prgRom[STAGE_INIT_TBL + (i * 3) + 0];
        data->stages[i].xPos.f.l = 0x80;
        data->stages[i].yPos.f.h = prgRom[STAGE_INIT_TBL + (i * 3) + 1];
        data->stages[i].yPos.f.l = 0x80;
        data->stages[i].roomNum  = prgRom[STAGE_INIT_TBL + (i * 3) + 2];
        data->stages[i].bossObj = prgRom[BOSS_TYPE_TBL + i];
        data->stages[i].bossSpawnCount = (i == 9) ? 12 : 1;
        data->stages[i].bossObjCount = prgRom[BOSS_COUNT_TBL + i];
    }

    // load warp door data
    #define WARP_DOOR_X_TBL (0x401a)
    #define WARP_DOOR_Y_TBL (0x412a)
    #define WARP_DOOR_ROOM_TBL (0x423a)
    
    data->numWarpDoors = 272;
    data->warpDoors = ommalloc(data->numWarpDoors * sizeof(WarpDoor));
    for (int i = 0; i < data->numWarpDoors; i++) {
        data->warpDoors[i].xPos = prgRom[WARP_DOOR_X_TBL + i];
        data->warpDoors[i].yPos = prgRom[WARP_DOOR_Y_TBL + i];
        data->warpDoors[i].roomNum = prgRom[WARP_DOOR_ROOM_TBL + i];
    }
    return data;
}

MapData *Rom_GetMapDataArcade(void) {
    MapData *data = Rom_GetMapData();
    // room 0's palette is changed to be the same as room 1's
    memcpy(data->rooms[0].palette, data->rooms[1].palette, sizeof(data->rooms[0].palette));

    // erase room 3's boss door
    data->screens[0x49][1] = 0x41;
    data->screens[0x49][5] = 0x44;
    // move room 3's boss door to the bottom of the room
    data->rooms[3].screenNums[56] = 0x31;
    // modify door table to reflect the moved door
    data->warpDoors[66].xPos = 5;
    data->warpDoors[66].yPos = 119;

    // NOTE: the arcade version  changed one of room 5's screens but it looks
    // really bad so I decided not to do that change here. If you want to see
    // it, uncomment this line:
    // data->rooms[5].screenNums[7] = 0x1f;

    // room 8's palette is changed
    Uint8 room8Pal[16] = {
        0x0f, 0x26, 0x06, 0x0c,
        0x0f, 0x1a, 0x09, 0x07,
        0x0f, 0x3b, 0x30, 0x00,
        0x0f, 0x24, 0x13, 0x0c,
    };
    memcpy(data->rooms[8].palette, room8Pal, sizeof(room8Pal));

    // room 11's palette is changed
    Uint8 room11Pal[16] = {
        0x0f, 0x0a, 0x06, 0x09,
        0x0f, 0x0c, 0x1c, 0x0c,
        0x0f, 0x1c, 0x32, 0x11,
        0x0f, 0x28, 0x07, 0x0f,
    };
    memcpy(data->rooms[11].palette, room11Pal, sizeof(room11Pal));

    // room 14's palette is changed
    Uint8 room14Pal[16] = {
        0x0f, 0x28, 0x00, 0x0f,
        // 0x2e is a 2c04 exclusive color
        0x0f, 0x2e, 0x0f, 0x09,
        0x0f, 0x1b, 0x32, 0x00,
        0x0f, 0x20, 0x10, 0x00,
    };
    memcpy(data->rooms[14].palette, room14Pal, sizeof(room14Pal));
    // don't spawn enemies around ending door (necessary due to changed spawning
    // behavior after daltos is killed)
    data->rooms[14].spawns[2].count = 0;
    data->rooms[14].spawns[4].count = 0;
    return data;
}
