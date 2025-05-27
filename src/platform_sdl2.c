/* platform_sdl2.c: SDL2 specific code
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

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "db.h"
#include "file.h"
#include "game.h"
#include "graphics.h"
#include "input.h"
#include "nanotime.h"
#include "nes_ntsc.h"
#include "platform.h"
#include "util.h"

// --- video stuff ---
static Uint8 frameStarted = 0;
static Uint8 scale = 3;
static Uint8 fullscreen = 0;
static int display = 0;
// NES framebuffer
static Uint8 framebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];
// destination to draw to when drawing in fullscreen
static SDL_Rect fullscreenRect;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
// texture that the game engine draws to
static SDL_Texture *drawTexture = NULL;
// texture that gets nearest-neighbor scaled
static SDL_Texture *scaleTexture = NULL;
static int vsync;
static nanotime_step_data stepData;
static nes_ntsc_t ntsc;
static Uint8 ntscEnabled = 0;

// --- audio stuff ---
static SDL_AudioDeviceID audioDevice;

// --- palette stuff ---
#define NUM_COLORS 64
// these are in ARGB 32bpp format that SDL likes
static Uint32 nesPalette[NUM_COLORS];
static Uint32 arcadePalette[NUM_COLORS];
// this is in RGB format that nes_ntsc likes
static Uint8 arcadePaletteNTSC[NUM_COLORS * 3];
static Uint8 paletteType;

// --- controller stuff ---
static const int gamepadMap[] = {
    [SDL_CONTROLLER_BUTTON_A] = INPUT_GAMEPAD_A,
    [SDL_CONTROLLER_BUTTON_B] = INPUT_GAMEPAD_B,
    [SDL_CONTROLLER_BUTTON_X] = INPUT_GAMEPAD_X,
    [SDL_CONTROLLER_BUTTON_Y] = INPUT_GAMEPAD_Y,
    [SDL_CONTROLLER_BUTTON_BACK] = INPUT_GAMEPAD_SELECT,
    [SDL_CONTROLLER_BUTTON_GUIDE] = INPUT_GAMEPAD_HOME,
    [SDL_CONTROLLER_BUTTON_START] = INPUT_GAMEPAD_START,
    [SDL_CONTROLLER_BUTTON_LEFTSTICK] = INPUT_GAMEPAD_L_STICK_PRESS,
    [SDL_CONTROLLER_BUTTON_RIGHTSTICK] = INPUT_GAMEPAD_R_STICK_PRESS,
    [SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = INPUT_GAMEPAD_L_SHOULDER,
    [SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = INPUT_GAMEPAD_R_SHOULDER,
    [SDL_CONTROLLER_BUTTON_DPAD_UP] = INPUT_GAMEPAD_DPAD_UP,
    [SDL_CONTROLLER_BUTTON_DPAD_DOWN] = INPUT_GAMEPAD_DPAD_DOWN,
    [SDL_CONTROLLER_BUTTON_DPAD_LEFT] = INPUT_GAMEPAD_DPAD_LEFT,
    [SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = INPUT_GAMEPAD_DPAD_RIGHT,
    [SDL_CONTROLLER_BUTTON_MAX] = INPUT_INVALID,
};
static SDL_GameController *controller;

// static function declarations
static void Platform_PumpEvents(void);
static int Platform_SetupRenderer(void);

static int Platform_InitVideo(void) {
    SDL_DisplayMode displayMode;
    SDL_GetDesktopDisplayMode(display, &displayMode);

    // set up window
    int windowWidth, windowHeight;
    if (fullscreen) {
        windowWidth = displayMode.w;
        windowHeight = displayMode.h;
    }
    else {
        windowWidth = ((int)(SCREEN_WIDTH * scale * PIXEL_ASPECT_RATIO));
        windowHeight = SCREEN_HEIGHT * scale;
    }
    window = SDL_CreateWindow(
        "OpenMadoola",
        SDL_WINDOWPOS_UNDEFINED_DISPLAY(display),
        SDL_WINDOWPOS_UNDEFINED_DISPLAY(display),
        windowWidth, windowHeight,
        fullscreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_SHOWN);
    if (!window) {
        Platform_ShowError("Error creating window: %s", SDL_GetError());
        return 0;
    }
    SDL_ShowCursor(fullscreen ? SDL_DISABLE : SDL_ENABLE);
    return Platform_SetupRenderer();
}

static int Platform_SetupRenderer(void) {
    SDL_DisplayMode displayMode;
    SDL_GetDesktopDisplayMode(display, &displayMode);

    if (drawTexture) { SDL_DestroyTexture(drawTexture); }
    if (scaleTexture) { SDL_DestroyTexture(scaleTexture); }
    if (renderer) { SDL_DestroyRenderer(renderer); }

    int refreshRate = displayMode.refresh_rate;
    // round up if refresh rate is 59 or something
    if (refreshRate && (((refreshRate + 1) % 60) == 0)) {
        refreshRate++;
    }
    if (refreshRate && ((refreshRate % 60) == 0)) {
        vsync = refreshRate / 60;
    }
    else {
        vsync = 0;
    }
    renderer = SDL_CreateRenderer(window, -1, vsync ? SDL_RENDERER_PRESENTVSYNC : 0);
    if (!renderer) {
        Platform_ShowError("Error creating renderer: %s", SDL_GetError());
        return 0;
    }
    nanotime_step_init(&stepData, (uint64_t)(NANOTIME_NSEC_PER_SEC / 60), nanotime_now_max(), nanotime_now, nanotime_sleep);

    // set up textures
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    int drawWidth;
    if (ntscEnabled) {
        drawWidth = NES_NTSC_OUT_WIDTH(SCREEN_WIDTH);
    }
    else {
        drawWidth = SCREEN_WIDTH;
    }
    drawTexture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    drawWidth, SCREEN_HEIGHT);
    if (!drawTexture) {
        Platform_ShowError("Error creating drawTexture: %s", SDL_GetError());
        return 0;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    int scaledWidth, scaledHeight;
    if (fullscreen) {
        int fullscreenScale = displayMode.h / SCREEN_HEIGHT;
        float fractionalScale = (float)displayMode.h / SCREEN_HEIGHT;
        scaledWidth = fullscreenScale * SCREEN_WIDTH;
        scaledHeight = fullscreenScale * SCREEN_HEIGHT;
        fullscreenRect.w = (int)(fractionalScale * SCREEN_WIDTH * PIXEL_ASPECT_RATIO);
        fullscreenRect.h = (int)(fractionalScale * SCREEN_HEIGHT);
        fullscreenRect.x = (displayMode.w / 2) - (fullscreenRect.w / 2);
        fullscreenRect.y = (displayMode.h / 2) - (fullscreenRect.h / 2);
    }
    else {
        scaledWidth = scale * SCREEN_WIDTH;
        scaledHeight = scale * SCREEN_HEIGHT;
    }
    scaleTexture = SDL_CreateTexture(renderer,
                                     SDL_PIXELFORMAT_ARGB8888,
                                     SDL_TEXTUREACCESS_TARGET,
                                     scaledWidth, scaledHeight);
    if (!scaleTexture) {
        Platform_ShowError("Error creating scaleTexture: %s", SDL_GetError());
        return 0;
    }
    return 1;
}

static void Platform_DestroyVideo(void) {
    SDL_DestroyTexture(drawTexture);  drawTexture = NULL;
    SDL_DestroyTexture(scaleTexture); scaleTexture = NULL;
    SDL_DestroyRenderer(renderer);    renderer = NULL;
    SDL_DestroyWindow(window);        window = NULL;
}

void Platform_StartFrame(void) {
    if (frameStarted) {
        printf("ERROR: Started frame without ending the previous frame!\n");
    }
    frameStarted = 1;
    Platform_PumpEvents();
}

void Platform_EndFrame(void) {
    static int burstPhase = 0;

    if (!frameStarted) {
        printf("ERROR: Ended frame without starting it!\n");
    }
    frameStarted = 0;
    // convert framebuffer from nes colors to rgb
    Uint32 *rgbFramebuffer;
    int pitch;
    SDL_LockTexture(drawTexture, NULL, (void **)&rgbFramebuffer, &pitch);
    if (ntscEnabled) {
        nes_ntsc_blit(&ntsc,
            framebuffer + (TILE_HEIGHT * FRAMEBUFFER_WIDTH) + TILE_WIDTH,
            FRAMEBUFFER_WIDTH,
            burstPhase,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            (void *)rgbFramebuffer,
            pitch);
        burstPhase ^= 1;
    }
    else {
        // skip past buffer around framebuffer
        int srcOffset = (TILE_HEIGHT * FRAMEBUFFER_WIDTH) + TILE_WIDTH;
        int dstOffset = 0;
        Uint32 *rgbPalette = (paletteType == PALETTE_TYPE_NES) ? nesPalette : arcadePalette;
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                rgbFramebuffer[dstOffset++] = rgbPalette[framebuffer[srcOffset++]];
            }
            srcOffset += (TILE_WIDTH * 2);
        }
    }
    SDL_UnlockTexture(drawTexture);
    SDL_SetRenderTarget(renderer, scaleTexture);
    // stretch framebuffer horizontally w/ bilinear so the pixel aspect ratio is correct
    SDL_RenderCopy(renderer, drawTexture, NULL, NULL);
    SDL_SetRenderTarget(renderer, NULL);

    // monitor framerate isn't a multiple of 60, so wait in software
    if (!vsync) {
        nanotime_step(&stepData);
    }

    for (int i = 0; i < (vsync ? vsync : 1); i++) {
        SDL_RenderClear(renderer);
        if (fullscreen) {
            SDL_RenderCopy(renderer, scaleTexture, NULL, &fullscreenRect);
        }
        else {
            SDL_RenderCopy(renderer, scaleTexture, NULL, NULL);
        }
        SDL_RenderPresent(renderer);
    }
}

void Platform_ShowError(char *fmt, ...) {
    char buff[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
    va_end(args);
    fprintf(stderr, "%s\n", buff);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", buff, window);
}

Uint8 *Platform_GetFramebuffer(void) {
    return framebuffer;
}

int Platform_GetVideoScale(void) {
    return scale;
}

int Platform_SetVideoScale(int requested) {
    if ((requested != scale) && (requested > 0) && !fullscreen) {
        scale = requested;
        // resize window
        int windowWidth = ((int)(SCREEN_WIDTH * scale * PIXEL_ASPECT_RATIO));
        int windowHeight = SCREEN_HEIGHT * scale;
        SDL_SetWindowSize(window, windowWidth, windowHeight);
        SDL_SetWindowPosition(window,
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED);
        Platform_SetupRenderer();
        DB_Set("scale", &scale, 1);
        DB_Save();
    }
    return scale;
}

int Platform_SetFullscreen(int requested) {
    if (requested != fullscreen) {
        fullscreen = requested;
        if (fullscreen) {
            SDL_DisplayMode displayMode;
            SDL_GetDesktopDisplayMode(display, &displayMode);
            SDL_SetWindowSize(window, displayMode.w, displayMode.h);
            SDL_SetWindowPosition(window,
                      SDL_WINDOWPOS_UNDEFINED_DISPLAY(display),
                      SDL_WINDOWPOS_UNDEFINED_DISPLAY(display));
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
            SDL_ShowCursor(SDL_DISABLE);
        }
        else {
            int windowWidth = ((int)(SCREEN_WIDTH * scale * PIXEL_ASPECT_RATIO));
            int windowHeight = SCREEN_HEIGHT * scale;
            SDL_SetWindowFullscreen(window, 0);
            SDL_SetWindowSize(window, windowWidth, windowHeight);
            SDL_SetWindowPosition(window,
                                  SDL_WINDOWPOS_CENTERED_DISPLAY(display),
                                  SDL_WINDOWPOS_CENTERED_DISPLAY(display));
            SDL_ShowCursor(SDL_ENABLE);
        }
        Platform_SetupRenderer();
        DB_Set("fullscreen", &fullscreen, 1);
        if (fullscreen) {
            Uint8 data[4];
            Util_SaveSint32(display, data);
            DB_Set("fullscreenDisplay", data, sizeof(data));
        }
        DB_Save();
    }
    return fullscreen;
}

int Platform_GetFullscreen(void) {
    return fullscreen;
}

static void Platform_InitNTSC(void) {
    nes_ntsc_setup_t ntscSetup = nes_ntsc_composite;
    ntscSetup.saturation = -0.1;
    // Sony CXA2025AS decoder matrix
    float matrix[6] = { 1.630f, 0.317f, -0.378f, -0.466f, -1.089f, 1.677f };
    ntscSetup.decoder_matrix = matrix;
    ntscSetup.base_palette = (paletteType == PALETTE_TYPE_2C04) ? arcadePaletteNTSC : NULL;
    nes_ntsc_init(&ntsc, &ntscSetup);
}

int Platform_SetNTSC(int requested) {
    if (requested != ntscEnabled) {
        ntscEnabled = requested;
        Platform_SetupRenderer();
        DB_Set("ntsc", &ntscEnabled, 1);
        DB_Save();
    }
    return ntscEnabled;
}

int Platform_GetNTSC(void) {
    return ntscEnabled;
}

void Platform_SetPaletteType(Uint8 type) {
    if (paletteType != type) {
        paletteType = type;
        Platform_InitNTSC();
    }
}

static int Platform_InitAudio(void) {
    SDL_AudioSpec spec = { 0 };
    spec.freq = 44100;
    spec.format = AUDIO_S16;
    spec.channels = 1;
    audioDevice = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    if (!audioDevice) {
        Platform_ShowError("Error creating audioDevice: %s", SDL_GetError());
        return 0;
    }
    SDL_PauseAudioDevice(audioDevice, 0);
    return 1;
}

static void Platform_DestroyAudio(void) {
    SDL_CloseAudioDevice(audioDevice);
}

void Platform_QueueSamples(Sint16 *samples, int count) {
    SDL_QueueAudio(audioDevice, (void *)samples, count * sizeof(Sint16));
}

int Platform_GetQueuedSamples(void) {
    return (int)SDL_GetQueuedAudioSize(audioDevice) / sizeof(Sint16);
}

int Platform_GamepadConnected(void) {
    return !!controller;
}

static int Platform_LoadPalette(char *filename, Uint32 *out, Uint8 *outNtsc) {
    FILE *fp = File_OpenResource(filename, "rb");
    if (!fp) {
        Platform_ShowError("Couldn't open %s", filename);
        return 0;
    }

    for (int i = 0; i < NUM_COLORS; i++) {
        Uint8 r = fgetc(fp);
        Uint8 g = fgetc(fp);
        Uint8 b = fgetc(fp);

        out[i] = ((Uint32)0xFF << 24) | ((Uint32)r << 16) | ((Uint32)g << 8) | (Uint32)b;
        if (outNtsc) {
            outNtsc[(i * 3) + 0] = r;
            outNtsc[(i * 3) + 1] = g;
            outNtsc[(i * 3) + 2] = b;
        }
    }
    fclose(fp);
    return 1;
}

static int Platform_InitPalettes(void) {
    if (!Platform_LoadPalette("nes.pal", nesPalette, NULL)) { return 0; }
    if (!Platform_LoadPalette("2c04.pal", arcadePalette, arcadePaletteNTSC)) { return 0; }
    return 1;
}

static SDL_GameController *Platform_FindController(void) {
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            return SDL_GameControllerOpen(i);
        }
    }

    return NULL;
}

int Platform_Init(void) {
    // this makes sure the application icon is correct
#ifdef OM_UNIX
    setenv("SDL_VIDEO_X11_WMCLASS", "com.infochunk.OpenMadoola", 1);
    setenv("SDL_VIDEO_WAYLAND_WMCLASS", "com.infochunk.OpenMadoola", 1);
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return 0;
    }
    DBEntry *entry = DB_Find("scale");
    if (entry) { scale = entry->data[0]; }
    entry = DB_Find("ntsc");
    if (entry) { ntscEnabled = entry->data[0]; }
    entry = DB_Find("fullscreen");
    if (entry) { fullscreen = entry->data[0]; }
    paletteType = (gameType == GAME_TYPE_ARCADE) ? PALETTE_TYPE_2C04 : PALETTE_TYPE_NES;
    entry = DB_Find("fullscreenDisplay");
    // only start on non-primary display if we're starting in fullscreen
    if (fullscreen && entry && (entry->dataLen == sizeof(Sint32))) {
        display = Util_LoadSint32(entry->data);
        // make sure display number is valid
        if (display >= SDL_GetNumVideoDisplays()) {
            display = 0;
        }
    }

    if (!Platform_InitPalettes()) { return 0; }
    Platform_InitNTSC();
    if (!Platform_InitVideo()) { return 0; }
    if (!Platform_InitAudio()) { return 0; }
    controller = Platform_FindController();
    return 1;
}

void Platform_Quit(void) {
    Platform_DestroyVideo();
    Platform_DestroyAudio();
    SDL_Quit();
    exit(0);
}

static void Platform_PumpEvents(void) {
    SDL_Event event;
    int button;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        // keyboard events
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            // alt+enter = toggle fullscreen
            if ((event.type == SDL_KEYDOWN) &&
                (event.key.keysym.scancode == SDL_SCANCODE_RETURN) &&
                (event.key.keysym.mod & KMOD_ALT)) {
                Platform_SetFullscreen(!Platform_GetFullscreen());
                break;
            }

            button = event.key.keysym.scancode;
            // these are discontiguous with the rest of the scancodes so we make them
            // match the button enum in input.h
            if ((button >= SDL_SCANCODE_LCTRL) && (button <= SDL_SCANCODE_RGUI)) {
                button = (button - SDL_SCANCODE_LCTRL) + INPUT_KEY_LCTRL;
            }
            if ((button >= INPUT_KEY_A) && (button <= INPUT_KEY_RGUI)) {
                Input_SetState(button, event.type == SDL_KEYDOWN);
            }
            break;

        // controller events
        case SDL_CONTROLLERDEVICEADDED:
            controller = SDL_GameControllerOpen(event.cdevice.which);
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
            controller = Platform_FindController();
            break;

        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            if (event.cbutton.button < ARRAY_LEN(gamepadMap)) {
                // match the button to the enum in input.h
                button = gamepadMap[event.cbutton.button];
                if (button != INPUT_INVALID) {
                    Input_SetState(button, event.type == SDL_CONTROLLERBUTTONDOWN);
                }
            }
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_DISPLAY_CHANGED) {
                display = event.window.data1;
                // reconfigure renderer because vsync status may have changed
                Platform_SetupRenderer();
            }
            break;

        // handle quit
        case SDL_QUIT:
            Platform_Quit();
            break;

        }
    }
}
