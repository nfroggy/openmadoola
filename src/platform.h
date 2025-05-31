/* platform.h: Platform specific code
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
#include "constants.h"
#include "graphics.h"

/**
 * @brief Initializes platform specific code.
 * @returns nonzero on success, zero on failure.
 */
int Platform_Init(void);

/**
 * @brief Shuts down platform code and exits the program.
 */
void Platform_Quit(void);

/**
 * @brief Displays an error message to the user
 * @param fmt test to print
 */
#ifdef __GNUC__
__attribute__((__format__(__printf__, 1, 2)))
#endif
void Platform_ShowError(char *fmt, ...);

/**
 * @brief Should be run at the start of each frame.
 */
void Platform_StartFrame(void);

/**
 * @brief Should be run at the end of each frame.
 */
void Platform_EndFrame(void);

/**
 * @brief Gets access to the framebuffer.
 * @returns a pointer to the NES framebuffer pixels (NES 6-bit).
 * NOTE: If you're implementing your own platform_whatever.c file, the
 * framebuffer should have a 1-tile wide buffer around each edge. This allows
 * for Graphics_DrawTile to skip checking if each pixel it's drawing is within
 * the bounds of the framebuffer.
 */
#define FRAMEBUFFER_WIDTH (SCREEN_WIDTH + (2 * TILE_WIDTH))
#define FRAMEBUFFER_HEIGHT (SCREEN_HEIGHT + (2 * TILE_HEIGHT))
Uint8 *Platform_GetFramebuffer(void);

/**
 * @returns the current video scale
 */
int Platform_GetVideoScale(void);

/**
 * @brief Requests changing the video scale. Doesn't work in fullscreen mode.
 * @param requested The requested scale
 * @returns The value the scale was set to
 */
int Platform_SetVideoScale(int requested);

/**
 * @brief Requests setting the video mode
 * @param requested nonzero = fullscreen, zero = windowed
 * @returns the set video mode
 */
int Platform_SetFullscreen(int requested);

/**
 * @brief Gets the video mode
 * @returns nonzero = fullscreen, zero = windowed
 */
int Platform_GetFullscreen(void);

/**
 * @brief Requests enabling or disabling the NTSC filter.
 * @param requested nonzero = filter enabled, zero = filter disabled
 * @returns the set filter mode
 */
int Platform_SetNTSC(int requested);

/**
 * @brief Gets the NTSC filter state
 * @returns nonzero = filter enabled, zero = filter disabled
 */
int Platform_GetNTSC(void);

#define PALETTE_TYPE_NES 0
#define PALETTE_TYPE_2C04 1

/**
 * @brief Sets the palette type to use.
 * @param type either PALETTE_TYPE_NES or PALETTE_TYPE_2C04
 */
void Platform_SetPaletteType(Uint8 type);

/**
 * Gets arcade color correction state
 * @returns nonzero = color correction enabled, zero = color correction disabled
 */
int Platform_GetArcadeColor(void);

/**
 * @brief Sets arcade color correction state.
 * @param requested nonzero = color correction enabled, zero = color correction disabled
 * @return the set arcade color correction mode
 */
int Platform_SetArcadeColor(int requested);

/**
 * @brief Queues the given audio samples to play
 * @param samples The buffer of samples to queue
 * @param count The number of samples in the buffer
 */
void Platform_QueueSamples(Sint16 *samples, int count);

/**
 * @returns the number of queued audio samples
 */
int Platform_GetQueuedSamples(void);

/**
 * @returns 1 if there's a gamepad connected, 0 otherwise
 */
int Platform_GamepadConnected(void);