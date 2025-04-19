/* platform_sdl3.c: SDL3 specific code
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

#include <SDL3/SDL.h>
#include <math.h>
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
#include "palette.h"
#include "platform.h"
#include "util.h"

// --- video stuff ---
static Uint8 frameStarted = 0;
static Uint8 scale = 3;
static Uint8 fullscreen = 0;
static Uint32 display;
// NES framebuffer
static Uint8 framebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];
// destination to draw to when drawing in fullscreen
static SDL_FRect fullscreenRect;
static SDL_PropertiesID windowProperties;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
// texture that the game engine draws to
static SDL_Texture *drawTexture = NULL;
// texture that gets nearest-neighbor scaled
static SDL_Texture *scaleTexture = NULL;
static int vsync;
static nanotime_step_data stepData;
static nes_ntsc_t ntsc;
static nes_ntsc_setup_t ntscSetup;
static Uint8 ntscEnabled = 0;

// --- audio stuff ---
static SDL_AudioStream *audioStream;

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
    [SDL_GAMEPAD_BUTTON_SOUTH] = INPUT_GAMEPAD_A,
    [SDL_GAMEPAD_BUTTON_EAST] = INPUT_GAMEPAD_B,
    [SDL_GAMEPAD_BUTTON_WEST] = INPUT_GAMEPAD_X,
    [SDL_GAMEPAD_BUTTON_NORTH] = INPUT_GAMEPAD_Y,
    [SDL_GAMEPAD_BUTTON_BACK] = INPUT_GAMEPAD_SELECT,
    [SDL_GAMEPAD_BUTTON_GUIDE] = INPUT_GAMEPAD_HOME,
    [SDL_GAMEPAD_BUTTON_START] = INPUT_GAMEPAD_START,
    [SDL_GAMEPAD_BUTTON_LEFT_STICK] = INPUT_GAMEPAD_L_STICK_PRESS,
    [SDL_GAMEPAD_BUTTON_RIGHT_STICK] = INPUT_GAMEPAD_R_STICK_PRESS,
    [SDL_GAMEPAD_BUTTON_LEFT_SHOULDER] = INPUT_GAMEPAD_L_SHOULDER,
    [SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER] = INPUT_GAMEPAD_R_SHOULDER,
    [SDL_GAMEPAD_BUTTON_DPAD_UP] = INPUT_GAMEPAD_DPAD_UP,
    [SDL_GAMEPAD_BUTTON_DPAD_DOWN] = INPUT_GAMEPAD_DPAD_DOWN,
    [SDL_GAMEPAD_BUTTON_DPAD_LEFT] = INPUT_GAMEPAD_DPAD_LEFT,
    [SDL_GAMEPAD_BUTTON_DPAD_RIGHT] = INPUT_GAMEPAD_DPAD_RIGHT,
    [SDL_GAMEPAD_BUTTON_COUNT] = INPUT_INVALID,
};
static SDL_Gamepad *gamepad;

// static function declarations
static void Platform_PumpEvents(void);
static int Platform_SetupRenderer(void);

static int Platform_InitVideo(void) {
    const SDL_DisplayMode *displayMode = SDL_GetDesktopDisplayMode(display);
    // set up window
    int windowWidth, windowHeight;
    if (fullscreen) {
        windowWidth = displayMode->w;
        windowHeight = displayMode->h;
        SDL_HideCursor();
    }
    else {
        windowWidth = ((int)(SCREEN_WIDTH * scale * PIXEL_ASPECT_RATIO));
        windowHeight = SCREEN_HEIGHT * scale;
        SDL_ShowCursor();
    }

    windowProperties = SDL_CreateProperties();
    if (!windowProperties) {
        Platform_ShowError("Error creating window properties: %s", SDL_GetError());
        return 0;
    }

    SDL_SetStringProperty(windowProperties, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "OpenMadoola");
    SDL_SetNumberProperty(windowProperties, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, windowWidth);
    SDL_SetNumberProperty(windowProperties, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, windowHeight);
    SDL_SetBooleanProperty(windowProperties, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, (bool)fullscreen);
    SDL_SetNumberProperty(windowProperties, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_UNDEFINED_DISPLAY(display));
    SDL_SetNumberProperty(windowProperties, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_UNDEFINED_DISPLAY(display));

    window = SDL_CreateWindowWithProperties(windowProperties);
    if (!window) {
        Platform_ShowError("Error creating window: %s", SDL_GetError());
        return 0;
    }

    return Platform_SetupRenderer();
}

static int Platform_SetupRenderer(void) {
    const SDL_DisplayMode *displayMode = SDL_GetDesktopDisplayMode(display);

    if (drawTexture) { SDL_DestroyTexture(drawTexture); }
    if (scaleTexture) { SDL_DestroyTexture(scaleTexture); }
    if (renderer) { SDL_DestroyRenderer(renderer); }

    // set up renderer
    int refreshRate = (int)(displayMode->refresh_rate);
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
    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        Platform_ShowError("Error creating renderer: %s", SDL_GetError());
        return 0;
    }
    if (vsync) {
        if (!SDL_SetRenderVSync(renderer, 1)) {
            // failed to enable vsync so fall back to software delay
            vsync = 0;
        }
    }
    nanotime_step_init(&stepData, (uint64_t)(NANOTIME_NSEC_PER_SEC / 60), nanotime_now_max(), nanotime_now, nanotime_sleep);

    // set up textures
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
    SDL_SetTextureScaleMode(drawTexture, SDL_SCALEMODE_NEAREST);
    int scaledWidth, scaledHeight;
    if (fullscreen) {
        int fullscreenScale = displayMode->h / SCREEN_HEIGHT;
        scaledWidth = fullscreenScale * SCREEN_WIDTH;
        scaledHeight = fullscreenScale * SCREEN_HEIGHT;

        float fractionalScale = (float)displayMode->h / SCREEN_HEIGHT;
        fullscreenRect.w = floorf((fractionalScale * SCREEN_WIDTH * PIXEL_ASPECT_RATIO));
        fullscreenRect.h = (float)displayMode->h;
        fullscreenRect.x = floorf((displayMode->w / 2) - (fullscreenRect.w / 2));
        fullscreenRect.y = 0;
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
    SDL_DestroyTexture(drawTexture);    drawTexture = NULL;
    SDL_DestroyTexture(scaleTexture);   scaleTexture = NULL;
    SDL_DestroyRenderer(renderer);      renderer = NULL;
    SDL_DestroyWindow(window);          window = NULL;
    SDL_DestroyProperties(windowProperties);
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
    Uint32 *rgbFramebuffer = NULL;
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
    SDL_RenderTexture(renderer, drawTexture, NULL, NULL);
    SDL_SetRenderTarget(renderer, NULL);

    // monitor framerate isn't a multiple of 60, so wait in software
    if (!vsync) {
        nanotime_step(&stepData);
    }

    for (int i = 0; i < (vsync ? vsync : 1); i++) {
        SDL_RenderClear(renderer);
        if (fullscreen) {
            SDL_RenderTexture(renderer, scaleTexture, NULL, &fullscreenRect);
        }
        else {
            SDL_RenderTexture(renderer, scaleTexture, NULL, NULL);
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
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", buff, window);
}

Uint8 *Platform_GetFramebuffer(void) {
    return framebuffer;
}

int Platform_GetVideoScale(void) {
    return scale;
}

int Platform_SetVideoScale(int requested) {
    if ((requested > 0) && !fullscreen) {
        scale = requested;
        // resize window
        int windowWidth = ((int)(SCREEN_WIDTH * scale * PIXEL_ASPECT_RATIO));
        int windowHeight = SCREEN_HEIGHT * scale;
        SDL_SetWindowSize(window, windowWidth, windowHeight);
        SDL_SetWindowPosition(window,
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED);
        // resize scale framebuffer to fit window
        SDL_DestroyTexture(scaleTexture);
        int scaledWidth = SCREEN_WIDTH * scale;
        int scaledHeight = SCREEN_HEIGHT * scale;
        scaleTexture = SDL_CreateTexture(renderer,
                                         SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_TARGET,
                                         scaledWidth, scaledHeight);
        DB_Set("scale", &scale, 1);
        DB_Save();
    }
    return scale;
}

int Platform_SetFullscreen(int requested) {
    if (requested != fullscreen) {
        fullscreen = requested;
        Platform_DestroyVideo();
        Platform_InitVideo();
        DB_Set("fullscreen", &fullscreen, 1);
        if (fullscreen) {
            Uint8 data[4];
            Util_SaveUint32(display, data);
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
    ntscSetup = nes_ntsc_composite;
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
        SDL_DestroyTexture(drawTexture);
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
            Platform_ShowError("Error recreating drawTexture: %s", SDL_GetError());
            abort();
            return requested;
        }
        SDL_SetTextureScaleMode(drawTexture, SDL_SCALEMODE_NEAREST);
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
    spec.format = SDL_AUDIO_S16;
    spec.channels = 1;
    audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    if (!audioStream) {
        Platform_ShowError("Error creating audioStream: %s", SDL_GetError());
        return 0;
    }
    SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(audioStream));
    return 1;
}

static void Platform_DestroyAudio(void) {
    SDL_DestroyAudioStream(audioStream);
}

void Platform_QueueSamples(Sint16 *samples, int count) {
    SDL_PutAudioStreamData(audioStream, (void *)samples, count * sizeof(Sint16));
}

int Platform_GetQueuedSamples(void) {
    return (int)SDL_GetAudioStreamQueued(audioStream) / sizeof(Sint16);
}

int Platform_GamepadConnected(void) {
    return !!gamepad;
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

static SDL_Gamepad *Platform_FindGamepad(void) {
    int numGamepads;
    SDL_JoystickID *gamepadIDs = SDL_GetGamepads(&numGamepads);
    if (gamepadIDs && numGamepads) {
        SDL_Gamepad *newGamepad = SDL_OpenGamepad(gamepadIDs[0]);
        SDL_free(gamepadIDs);
        return newGamepad;
    }
    return NULL;
}

int Platform_Init(void) {
    // this makes sure the application icon is correct
    SDL_SetHint(SDL_HINT_APP_ID, "com.infochunk.OpenMadoola");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD)) {
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
    if (fullscreen && entry && (entry->dataLen == sizeof(Uint32))) {
        display = Util_LoadUint32(entry->data);
        // make sure display number is valid
        int numDisplays;
        SDL_DisplayID *displays = SDL_GetDisplays(&numDisplays);
        int found = 0;
        for (int i = 0; i < numDisplays; i++) {
            if (displays[i] == display) {
                found = 1;
                break;
            }
        }
        if (!found) {
            display = SDL_GetPrimaryDisplay();
        }
        SDL_free(displays);
    }
    else {
        display = SDL_GetPrimaryDisplay();
    }

    if (!Platform_InitPalettes()) { return 0; }
    Platform_InitNTSC();
    if (!Platform_InitVideo()) { return 0; }
    if (!Platform_InitAudio()) { return 0; }
    gamepad = Platform_FindGamepad();
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
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            // alt+enter = toggle fullscreen
            if ((event.type == SDL_EVENT_KEY_DOWN) &&
                (event.key.scancode == SDL_SCANCODE_RETURN) &&
                (event.key.mod & SDL_KMOD_ALT)) {
                Platform_SetFullscreen(!Platform_GetFullscreen());
                break;
            }

            button = event.key.scancode;
            // these are discontiguous with the rest of the scancodes so we make them
            // match the button enum in input.h
            if ((button >= SDL_SCANCODE_LCTRL) && (button <= SDL_SCANCODE_RGUI)) {
                button = (button - SDL_SCANCODE_LCTRL) + INPUT_KEY_LCTRL;
            }
            if ((button >= INPUT_KEY_A) && (button <= INPUT_KEY_RGUI)) {
                Input_SetState(button, event.type == SDL_EVENT_KEY_DOWN);
            }
            break;

        // controller events
        case SDL_EVENT_GAMEPAD_ADDED:
            gamepad = SDL_OpenGamepad(event.gdevice.which);
            break;

        case SDL_EVENT_GAMEPAD_REMOVED:
            gamepad = Platform_FindGamepad();
            break;

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            if (event.gbutton.button < ARRAY_LEN(gamepadMap)) {
                // match the button to the enum in input.h
                button = gamepadMap[event.gbutton.button];
                if (button != INPUT_INVALID) {
                    Input_SetState(button, event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
                }
            }
            break;

        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
            display = (Uint32)(event.window.data1);
            // reconfigure renderer because vsync status may have changed
            Platform_SetupRenderer();
            break;

        // handle quit
        case SDL_EVENT_QUIT:
            Platform_Quit();
            break;

        }
    }
}
