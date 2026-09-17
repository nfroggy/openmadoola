  /* map.c: handles accessing map data
 * Copyright (c) 2023, 2024, 2026 Nathan Misner
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
#include "buffer.h"
#include "constants.h"
#include "file.h"
#include "graphics.h"
#include "map.h"
#include "object.h"
#include "palette.h"
#include "util.h"

MapData mapData;
Uint16 *mapMetatiles;
Uint8 currRoom = 0xff;
Uint8 roomWidthMetatiles, roomHeightMetatiles;
static Uint16 scrollX;
static Uint16 scrollY;

void Map_LoadData(const char *filename) {
    FILE *fp = File_OpenResource(filename, "rb");
    if (!fp) {
        printf("couldn't open file\n");
        return;
    }
    Buffer *fileBuf = Buffer_InitFromFile(fp);
    Uint8 *mapFile = fileBuf->data;

    if (memcmp(mapFile, "Lucia", 5) != 0) {
        printf("invalid map file\n");
        return;
    }

    Uint8 version = mapFile[5];
    if (version != 0) {
        printf("unsupported map version %u\n", version);
        return;
    }
    int cursor = 6;

    // load tilesets
    mapData.numTilesets = Util_LoadUint16(mapFile + cursor); cursor += 2;
    mapData.tilesets = ommalloc(mapData.numTilesets * sizeof(Tileset));
    for (Uint16 i = 0; i < mapData.numTilesets; i++) {
        Tileset *ts = &mapData.tilesets[i];

        // load metatile definitions
        ts->numMetatiles = Util_LoadUint16(mapFile + cursor); cursor += 2;
        ts->metatiles = ommalloc(ts->numMetatiles * sizeof(Metatile));
        for (Uint16 j = 0; j < ts->numMetatiles; j++) {
            Metatile *mt = &ts->metatiles[j];
            mt->type = (MetatileType)mapFile[cursor++];
            mt->palnum = mapFile[cursor++];
            for (int k = 0; k < ARRAY_LEN(mt->tiles); k++) {
                mt->tiles[k] = Util_LoadUint16(mapFile + cursor); cursor += 2;
            }
        }

        // load chunk definitions
        ts->numChunks = Util_LoadUint16(mapFile + cursor); cursor += 2;
        ts->chunks = ommalloc(ts->numChunks * sizeof(ts->chunks[0]));
        for (Uint16 j = 0; j < ts->numChunks; j++) {
            for (int k = 0; k < ARRAY_LEN(ts->chunks[0]); k++) {
                ts->chunks[j][k] = Util_LoadUint16(mapFile + cursor); cursor += 2;
            }
        }

        // load screen definitions
        ts->numScreens = Util_LoadUint16(mapFile + cursor); cursor += 2;
        ts->screens = ommalloc(ts->numScreens * sizeof(ts->screens[0]));
        for (Uint16 j = 0; j < ts->numScreens; j++) {
            for (int k = 0; k < ARRAY_LEN(ts->screens[0]); k++) {
                ts->screens[j][k] = Util_LoadUint16(mapFile + cursor); cursor += 2;
            }
        }
    }

    // load maps
    mapData.numMaps = Util_LoadUint16(mapFile + cursor); cursor += 2;
    mapData.maps = ommalloc(mapData.numMaps * sizeof(Map));
    for (Uint16 i = 0; i < mapData.numMaps; i++) {
        Map *map = &mapData.maps[i];

        map->tileset = Util_LoadUint16(mapFile + cursor); cursor += 2;
        map->song = mapFile[cursor++];
        memcpy(map->palette, mapFile + cursor, 16); cursor += 16;

        map->numPaletteAnims = mapFile[cursor++];
        map->paletteAnims = ommalloc(map->numPaletteAnims * sizeof(PaletteAnim));
        for (Uint16 j = 0; j < map->numPaletteAnims; j++) {
            PaletteAnim *pa = &map->paletteAnims[j];
            pa->startEntry = mapFile[cursor++];
            pa->numEntries = mapFile[cursor++];
            pa->numFrames = Util_LoadUint16(mapFile + cursor); cursor += 2;
            pa->frames = ommalloc(pa->numFrames * sizeof(PaletteAnimFrame));
            for (Uint16 k = 0; k < pa->numFrames; k++) {
                pa->frames[k].duration = Util_LoadUint16(mapFile + cursor); cursor += 2;
                pa->frames[k].colors = ommalloc(pa->numEntries);
                memcpy(pa->frames[k].colors, mapFile + cursor, pa->numEntries); cursor += pa->numEntries;
            }
        }

        map->width = mapFile[cursor++];
        map->height = mapFile[cursor++];
        map->scrollMode = mapFile[cursor++];
        map->screenNums = ommalloc(map->width * map->height * sizeof(Uint16));
        for (int j = 0; j < (map->width * map->height); j++) {
            map->screenNums[j] = Util_LoadUint16(mapFile + cursor); cursor += 2;
        }

        map->spawnMode = mapFile[cursor++];
        map->spawns = ommalloc(map->width * map->height * sizeof(SpawnInfo));
        for (int j = 0; j < (map->width * map->height); j++) {
            if (map->spawnMode == SPAWN_MODE_ENEMY) {
                map->spawns[j].enemy.id = Util_LoadUint16(mapFile + cursor); cursor += 2;
                map->spawns[j].enemy.count = mapFile[cursor++];
            }
            else {
                map->spawns[j].boss = mapFile[cursor++];
            }
        }

        map->numObjects = Util_LoadUint16(mapFile + cursor); cursor += 2;
        map->objects = ommalloc(map->numObjects * sizeof(ObjectSpawn));
        for (int j = 0; j < map->numObjects; j++) {
            ObjectSpawn *os = &map->objects[j];
            os->id = Util_LoadUint16(mapFile + cursor); cursor += 2;
            os->xPos.v = Util_LoadSint16(mapFile + cursor); cursor += 2;
            os->yPos.v = Util_LoadSint16(mapFile + cursor); cursor += 2;
            os->param = Util_LoadUint16(mapFile + cursor); cursor += 2;
        }
    }

    // door table
    mapData.numWarpDoors = Util_LoadUint16(mapFile + cursor); cursor += 2;
    mapData.warpDoors = ommalloc(mapData.numWarpDoors * sizeof(WarpDoor));
    for (int i = 0; i < mapData.numWarpDoors; i++) {
        WarpDoor *wd = &mapData.warpDoors[i];
        wd->xPos = mapFile[cursor++];
        wd->yPos = mapFile[cursor++];
        wd->mapNum = Util_LoadUint16(mapFile + cursor); cursor += 2;
    }

    // stage table
    mapData.numStages = Util_LoadUint16(mapFile + cursor); cursor += 2;
    mapData.stages = ommalloc(mapData.numStages * sizeof(StageInfo));
    for (int i = 0; i < mapData.numStages; i++) {
        StageInfo *si = &mapData.stages[i];
        si->xPos.v = Util_LoadSint16(mapFile + cursor); cursor += 2;
        si->yPos.v = Util_LoadSint16(mapFile + cursor); cursor += 2;
        si->roomNum = Util_LoadUint16(mapFile + cursor); cursor += 2;
        si->bossObj = Util_LoadUint16(mapFile + cursor); cursor += 2;
        si->bossSpawnCount = mapFile[cursor++];
        si->bossObjCount = mapFile[cursor++];
    }
}

void Map_Init(Uint8 roomNum) {
    if (roomNum == currRoom) { return; }

    currRoom = roomNum;
    Uint8 roomWidthScreens = mapData.maps[currRoom].width;
    Uint8 roomHeightScreens = mapData.maps[currRoom].height;
    Tileset *tileset = &mapData.tilesets[mapData.maps[currRoom].tileset];
    roomWidthMetatiles = roomWidthScreens * SCREEN_WIDTH_METATILES;
    roomHeightMetatiles = roomHeightScreens * SCREEN_HEIGHT_METATILES;
    if (mapMetatiles) { free(mapMetatiles); }
    mapMetatiles = ommalloc(roomWidthMetatiles * roomHeightMetatiles * sizeof(Uint16));

    // decompress the room's metatiles
    for (int screenY = 0; screenY < roomHeightScreens; screenY++) {
        for (int screenX = 0; screenX < roomWidthScreens; screenX++) {
            int screenNum = mapData.maps[roomNum].screenNums[screenY * roomWidthScreens + screenX];
            for (int chunkY = 0; chunkY < 4; chunkY++) {
                for (int chunkX = 0; chunkX < 4; chunkX++) {
                    int chunkNum = tileset->screens[screenNum][chunkY * 4 + chunkX];
                    for (int metatileY = 0; metatileY < 4; metatileY++) {
                        for (int metatileX = 0; metatileX < 4; metatileX++) {
                            Uint16 metatileNum = tileset->chunks[chunkNum][metatileY * 4 + metatileX];
                            int xPos = (screenX * 16) + (chunkX * 4) + metatileX;
                            int yPos = (screenY * 16) + (chunkY * 4) + metatileY;
                            mapMetatiles[yPos * roomWidthMetatiles + xPos] = metatileNum;
                        }
                    }
                }
            }
        }
    }

    // load the room's palettes
    Map_LoadPalettes(roomNum);
}

void Map_LoadPalettes(Uint8 roomNum) {
    memcpy(colorPalette, mapData.maps[roomNum].palette, sizeof(mapData.maps[roomNum].palette));
}

void Map_GetSpawnInfo(Object *o, SpawnInfo *info) {
    int xScreen = o->x.v >> 12;
    int yScreen = o->y.v >> 12;
    Map *map = &mapData.maps[currRoom];
    *info = map->spawns[yScreen * map->width + xScreen];
}

Uint16 Map_CheckX(Object *o) {
    Uint16 collision = o->collision;

    // less than halfway through the metatile
    if (o->x.f.l < 0x80) {
        // add "walls" around the map border to prevent from going off the map
        if (o->x.f.h <= 0) {
            goto found_tile;
        }
        // look for solid tiles in the previous metatile
        collision--;
    }
    // halfway or more through the metatile
    else {
        // add "walls" around the map border to prevent from going off the map
        if (o->x.f.h >= 0x7f) {
            goto found_tile;
        }
        // look for solid tiles in the next metatile
        collision++;
    }

    if (mapMetatiles[collision] < MAP_SOLID) {
        // solid tile, so make the object snap to the metatile boundary
        goto found_tile;
    }

    // if we didn't find any solid tiles and are in the upper half of the metatile, check up a metatile
    if (o->y.f.l < 0x80) {
        collision -= roomWidthMetatiles;
    }
    // if we're in the lower ~1/4 of the metatile, check down a metatile
    else if (o->y.f.l >= 0xa0) {
        collision += roomWidthMetatiles;
    }
    // otherwise, give up
    else {
        return 0;
    }

    if (mapMetatiles[collision] < MAP_SOLID) {
        goto found_tile;
    }
    return 0;

found_tile:
    // snap object to metatile boundary
    o->x.f.l = 0x80;
    return 1;

}

Uint16 Map_CheckY(Object *o) {
    Uint16 collision = o->collision;

    // less than halfway through the metatile
    if (o->y.f.l < 0x80) {
        // add "walls" around the map borders to prevent from going off the map
        if (o->y.f.h == 0) {
            goto found_tile;
        }

        // look in the previous metatile
        collision -= roomWidthMetatiles;
    }

    else {
        // prevent from going off the bottom of the map
        if (o->y.f.h == 0x7f) {
            goto found_tile;
        }

        // look in the next metatile
        collision += roomWidthMetatiles;
    }

    if (mapMetatiles[collision] < MAP_SOLID) {
        goto found_tile;
    }

    // if we're in the upper ~1/4 of the tile, check forward a tile
    if (o->x.f.l >= 0xa8) {
        collision++;
    }

    // if we're in the lower ~1/4 of the tile, check backward a tile
    else if (o->x.f.l < 0x58) {
        collision--;
    }

    // otherwise, give up
    else {
        return 0;
    }

    if (mapMetatiles[collision] < MAP_SOLID) {
        goto found_tile;
    }
    return 0;

found_tile:
    // snap object to metatile boundary
    o->y.f.l = 0x80;
    return 1;
}

Uint16 Map_SolidTileBelow(Uint16 offset) {
    Uint16 mapBound = roomWidthMetatiles * roomHeightMetatiles;
    if (offset >= mapBound) { return 1; }
    Uint16 metatile = mapMetatiles[offset];
    // are we on top of a solid tile or a ladder?
    if (metatile < MAP_LADDER) {
        // look down a metatile
        offset += roomWidthMetatiles;
        if (offset >= mapBound) { return 1; }
        metatile = mapMetatiles[offset];
        // are we on top of a solid tile (not a ladder)?
        if (metatile < MAP_SOLID) {
            return 1;
        }
    }
    else {
        offset += roomWidthMetatiles;
        if (offset >= mapBound) { return 1; }
        metatile = mapMetatiles[offset];
        // are we on top of a solid block or ladder? ladders are solid from the
        // top
        if (metatile < MAP_LADDER) {
            return 1;
        }
    }
    return 0;
}

  Uint16 Map_SolidTileAbove(Uint16 offset) {
    Uint16 mapBound = roomWidthMetatiles * roomHeightMetatiles;
      if (offset >= mapBound) { return 1; }
      Uint16 metatile = mapMetatiles[offset];
      // are we on top of a solid tile or a ladder?
      if (metatile < MAP_LADDER) {
          // look up a metatile
          offset -= roomWidthMetatiles;
          if (offset >= mapBound) { return 1; }
          metatile = mapMetatiles[offset];
          // are we on top of a solid tile (not a ladder)?
          if (metatile < MAP_SOLID) {
              return 1;
          }
      }
      else {
          offset -= roomWidthMetatiles;
          if (offset >= mapBound) { return 1; }
          metatile = mapMetatiles[offset];
          // are we on top of a solid block or ladder? ladders are solid from the
          // top
          if (metatile < MAP_LADDER) {
              return 1;
          }
      }
      return 0;
  }

int Map_Door(Object *o) {
    Uint8 chunkAlignedX = o->x.f.h & 0xfc;
    Uint8 chunkAlignedY = o->y.f.h & 0xfc;

    for (int i = 0; i < mapData.numWarpDoors; i++) {
        Uint8 chunkAlignedDoorX = mapData.warpDoors[i].xPos & 0xfc;
        Uint8 chunkAlignedDoorY = mapData.warpDoors[i].yPos & 0xfc;
        if ((mapData.warpDoors[i].mapNum == currRoom) &&
            (chunkAlignedDoorX == chunkAlignedX) &&
            (chunkAlignedDoorY == chunkAlignedY))
        {
                // ending door
                if (i == 0) {
                    return DOOR_ENDING;
                }
                int doorIndex = i ^ 1;
                o->x.f.l = 0x80;
                o->y.f.l = 0x80;
                o->x.f.h = mapData.warpDoors[doorIndex].xPos;
                o->y.f.h = mapData.warpDoors[doorIndex].yPos;
                return mapData.warpDoors[doorIndex].mapNum;
        }
    }

    return DOOR_INVALID;
}

void Map_SetPos(Uint16 x, Uint16 y) {
    scrollX = x;
    scrollY = y;
}

void Map_Draw(void) {
    Tileset *tileset = &mapData.tilesets[mapData.maps[currRoom].tileset];

    for (int y = 0; y < SCREEN_HEIGHT + METATILE_SIZE; y += METATILE_SIZE) {
        for (int x = 0; x < SCREEN_WIDTH + METATILE_SIZE; x += METATILE_SIZE) {
            int xPos = x - (scrollX % METATILE_SIZE);
            int yPos = y - (scrollY % METATILE_SIZE);

            int xTile = (((x + scrollX) / METATILE_SIZE) % roomWidthMetatiles);
            int yTile = (((y + scrollY) / METATILE_SIZE) % roomWidthMetatiles);

            Metatile *metatile = &tileset->metatiles[mapMetatiles[yTile * roomWidthMetatiles + xTile]];
            Graphics_DrawBGTile(xPos + 0, yPos + 0, metatile->tiles[0], metatile->palnum);
            Graphics_DrawBGTile(xPos + 8, yPos + 0, metatile->tiles[1], metatile->palnum);
            Graphics_DrawBGTile(xPos + 0, yPos + 8, metatile->tiles[2], metatile->palnum);
            Graphics_DrawBGTile(xPos + 8, yPos + 8, metatile->tiles[3], metatile->palnum);
        }
    }
}
