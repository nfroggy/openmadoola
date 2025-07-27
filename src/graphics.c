/* graphics.c: Graphics primitives
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
#include "constants.h"

#if defined(OM_AMD64)
#include <immintrin.h>
#endif
#if defined(_MSC_VER)
#include <isa_availability.h>
#endif

#include <stdio.h>
#include <string.h>

#include "alloc.h"
#include "graphics.h"
#include "palette.h"
#include "platform.h"
#include "rom.h"

#define TILE_PACKED_SIZE (16)
#define TILE_SIZE (TILE_WIDTH * TILE_HEIGHT)

// 8bpp chunky version of chrRom
static Uint8 *chrData;
// the palette we're using to draw this frame
static Uint8 *drawPalette;
// where we're drawing to
static Uint8 *screen;

void (*Graphics_DrawBGTile)(int x, int y, int tilenum, int palnum);
#if defined(OM_AMD64)
static void Graphics_DrawBGTileAVX2(int x, int y, int tilenum, int palnum);
#endif
static void Graphics_DrawBGTileFallback(int x, int y, int tilenum, int palnum);

int Graphics_Init(void) {
    // convert planar 2bpp to chunky 8bpp
    chrData = omaligned_alloc(32, chrRomSize * 4);
    int chrCursor = 0;
    for (int i = 0; i < (chrRomSize / TILE_PACKED_SIZE); i++) {
        int tilePos = i * TILE_PACKED_SIZE;
        // each tile is 8x8 pixels
        for (int y = 0; y < TILE_HEIGHT; y++) {
            uint8_t lowByte = chrRom[tilePos + y];
            uint8_t highByte = chrRom[tilePos + y + 8];
            for (int x = 0; x < TILE_WIDTH; x++) {
                uint8_t pixel = 0;
                if (lowByte & 0x80) {
                    pixel |= 1;
                }
                lowByte <<= 1;

                if (highByte & 0x80) {
                    pixel |= 2;
                }
                highByte <<= 1;

                chrData[chrCursor++] = pixel;
            }
        }
    }

#if defined(OM_AMD64)
#if defined(__GNUC__)
    if (__builtin_cpu_supports("avx2")) {
        Graphics_DrawBGTile = Graphics_DrawBGTileAVX2;
    }
    else {
        Graphics_DrawBGTile = Graphics_DrawBGTileFallback;
    }
#elif defined(_MSC_VER)
    if (__check_isa_support(__IA_SUPPORT_VECTOR256, 0)) {
        Graphics_DrawBGTile = Graphics_DrawBGTileAVX2;
    }
    else {
        Graphics_DrawBGTile = Graphics_DrawBGTileFallback;
    }
#else
    Graphics_DrawBGTile = Graphics_DrawBGTileFallback;
#endif // defined(OM_AMD64)
#else
    Graphics_DrawBGTile = Graphics_DrawBGTileFallback;
#endif
    return 1;
}

void Graphics_StartFrame(void) {
    screen = Platform_GetFramebuffer();
    drawPalette = Palette_Run();
    memset(screen, colorPalette[0], FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);
}

void Graphics_DrawTile(int x, int y, int tilenum, int palnum, int mirror) {
    // don't draw the tile at all if it's entirely offscreen
    if ((x < -TILE_WIDTH) || (x >= SCREEN_WIDTH) || (y < -TILE_HEIGHT) || (y >= SCREEN_HEIGHT)) {
        return;
    }

    // write tile to screen
    int tileOffset = tilenum * TILE_SIZE;
    Uint8 *palette = drawPalette + (palnum * PALETTE_SIZE);
    // the nes framebuffer has an extra tile row/column around it to allow for 
    // drawing to it without checking the tile bounds
    x += TILE_WIDTH;
    y += TILE_HEIGHT;

    switch (mirror) {
    case 0: // no mirror
        for (int yOffset = 0; yOffset < TILE_HEIGHT; yOffset++) {
            int yDst = y + yOffset;
            for (int xOffset = 0; xOffset < TILE_WIDTH; xOffset++) {
                Uint8 palIndex = chrData[tileOffset++];
                if (palIndex) {
                    int xDst = x + xOffset;
                    screen[yDst * FRAMEBUFFER_WIDTH + xDst] = palette[palIndex];
                }
            }
        }
        break;

    case H_MIRROR:
        for (int yOffset = 0; yOffset < TILE_HEIGHT; yOffset++) {
            int yDst = y + yOffset;
            for (int xOffset = 0; xOffset < TILE_WIDTH; xOffset++) {
                Uint8 palIndex = chrData[tileOffset++];
                if (palIndex) {
                    int xDst = ((TILE_WIDTH - 1) - xOffset) + x;
                    screen[yDst * FRAMEBUFFER_WIDTH + xDst] = palette[palIndex];
                }
            }
        }
        break;

    case V_MIRROR:
        for (int yOffset = 0; yOffset < TILE_HEIGHT; yOffset++) {
            int yDst = ((TILE_HEIGHT - 1) - yOffset) + y;
            for (int xOffset = 0; xOffset < TILE_WIDTH; xOffset++) {
                Uint8 palIndex = chrData[tileOffset++];
                if (palIndex) {
                    int xDst = x + xOffset;
                    screen[yDst * FRAMEBUFFER_WIDTH + xDst] = palette[palIndex];
                }
            }
        }
        break;

    case (H_MIRROR | V_MIRROR):
        for (int yOffset = 0; yOffset < TILE_HEIGHT; yOffset++) {
            int yDst = ((TILE_HEIGHT - 1) - yOffset) + y;
            for (int xOffset = 0; xOffset < TILE_WIDTH; xOffset++) {
                Uint8 palIndex = chrData[tileOffset++];
                if (palIndex) {
                    int xDst = ((TILE_WIDTH - 1) - xOffset) + x;
                    screen[yDst * FRAMEBUFFER_WIDTH + xDst] = palette[palIndex];
                }
            }
        }
        break;
    }
}

#if defined(OM_AMD64)
// GCC/clang need to be told they're allowed to use AVX2, MSVC doesn't
#if defined(__GNUC__)
__attribute__((target("avx2")))
#endif // defined(__GNUC__)
static void Graphics_DrawBGTileAVX2(int x, int y, int tilenum, int palnum) {
    // don't draw the tile at all if it's entirely offscreen
    if ((x < -TILE_WIDTH) || (x >= SCREEN_WIDTH) || (y < -TILE_HEIGHT) || (y >= SCREEN_HEIGHT)) {
        return;
    }
    // the framebuffer has an extra tile row/column around it to allow for drawing to it without
    // checking the tile bounds
    x += TILE_WIDTH;
    y += TILE_HEIGHT;
    int tileOffset = tilenum * TILE_SIZE;

    // broadcast load the 4 palette bytes into an __m256i
    __m256i palette = _mm256_set1_epi32(*(Uint32 *)(drawPalette + (palnum * PALETTE_SIZE)));
    // load the 8x8 tile into 2 __m256i's
    __m256i half1 = _mm256_load_si256((const __m256i *)(chrData + tileOffset));
    __m256i half2 = _mm256_load_si256((const __m256i *)(chrData + tileOffset + (TILE_WIDTH * 4)));
    // use the tile data as a shuffle mask to convert palette indices to NES color data
    half1 = _mm256_shuffle_epi8(palette, half1);
    half2 = _mm256_shuffle_epi8(palette, half2);

    // write the tile data line by line to the framebuffer
    Uint8 *dst = screen + (y * FRAMEBUFFER_WIDTH + x);
    *((Uint64 *)dst) = (Uint64)_mm256_extract_epi64(half1, 0); dst += FRAMEBUFFER_WIDTH;
    *((Uint64 *)dst) = (Uint64)_mm256_extract_epi64(half1, 1); dst += FRAMEBUFFER_WIDTH;
    *((Uint64 *)dst) = (Uint64)_mm256_extract_epi64(half1, 2); dst += FRAMEBUFFER_WIDTH;
    *((Uint64 *)dst) = (Uint64)_mm256_extract_epi64(half1, 3); dst += FRAMEBUFFER_WIDTH;
    *((Uint64 *)dst) = (Uint64)_mm256_extract_epi64(half2, 0); dst += FRAMEBUFFER_WIDTH;
    *((Uint64 *)dst) = (Uint64)_mm256_extract_epi64(half2, 1); dst += FRAMEBUFFER_WIDTH;
    *((Uint64 *)dst) = (Uint64)_mm256_extract_epi64(half2, 2); dst += FRAMEBUFFER_WIDTH;
    *((Uint64 *)dst) = (Uint64)_mm256_extract_epi64(half2, 3);
}
#endif // defined(OM_AMD64)

static void Graphics_DrawBGTileFallback(int x, int y, int tilenum, int palnum) {
    // don't draw the tile at all if it's entirely offscreen
    if ((x < -TILE_WIDTH) || (x >= SCREEN_WIDTH) || (y < -TILE_HEIGHT) || (y >= SCREEN_HEIGHT)) {
        return;
    }
    // the framebuffer has an extra tile row/column around it to allow for drawing to it without
    // checking the tile bounds
    x += TILE_WIDTH;
    y += TILE_HEIGHT;
    int tileOffset = tilenum * TILE_SIZE;
    Uint8 *palette = drawPalette + (palnum * PALETTE_SIZE);

    for (int yOffset = 0; yOffset < TILE_HEIGHT; yOffset++) {
        int fbOffset = ((y + yOffset) * FRAMEBUFFER_WIDTH) + x;
        for (int xOffset = 0; xOffset < TILE_WIDTH; xOffset++) {
            screen[fbOffset++] = palette[chrData[tileOffset++]];
        }
    }
}
