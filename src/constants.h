/* constants.h: Useful constants and macros
 * Copyright (c) 2023-2025 Nathan Misner
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

#pragma once
#include <stdint.h>
#include <stdlib.h>

typedef int8_t   Sint8;
typedef uint8_t  Uint8;
typedef int16_t  Sint16;
typedef uint16_t Uint16;
typedef int32_t  Sint32;
typedef uint32_t Uint32;
typedef int64_t  Sint64;
typedef uint64_t Uint64;

#define OPENMADOOLA_VERSION "1.22"

#define SCREEN_WIDTH (256) // original NES aspect ratio
//#define SCREEN_WIDTH (368) // approx. 16:9 aspect ratio
#define SCREEN_HEIGHT (240)

#define PIXEL_ASPECT_RATIO (8.0f / 7.0f)

#if (defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)))
#define OM_UNIX
#elif (defined(_WIN32) || defined(WIN32))
#define OM_WINDOWS
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define OM_AMD64
#endif

#define ARRAY_LEN(x) (sizeof(x) / sizeof(x[0]))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define CLAMP(x, low, high) do { if ((x) < (low)) { (x) = (low); } if ((x) > (high)) { (x) = (high); } } while(0)
#define OM_STRINGIFY(x) #x
#define OM_TOSTR(x) OM_STRINGIFY(x)

// this makes it easier to access the low/high bytes of the value
typedef union {
#ifdef OM_BIG_ENDIAN
    struct {
        Sint8 h;
        Uint8 l;
    };
#else
    struct {
        Uint8 l;
        Sint8 h;
    } f;
#endif
    Sint16 v;
} Fixed16;
