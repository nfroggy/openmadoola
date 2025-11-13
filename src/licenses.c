/* licenses.c: Displays software license information
* Copyright (c) 2025 Nathan Misner
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

#include "bg.h"
#include "constants.h"
#include "joy.h"
#include "licenses.h"
#include "mainmenu.h"
#include "menu.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"

static void displayLicense(const char *title, const char **pages, int numPages) {
    int page = 0;

    // wait a frame to make sure joyEdge doesn't come through from the last frame
    Task_Yield();

    while (1) {
        BG_Clear();

        if (joyEdge & (JOY_LEFT | JOY_UP)) {
            page = MAX(page - 1, 0);
            Sound_Play(SFX_MENU);
        }
        if (joyEdge & (JOY_RIGHT | JOY_DOWN )) {
            page = MIN(page + 1, numPages - 1);
            Sound_Play(SFX_MENU);
        }
        if (joyEdge & (JOY_A | JOY_START | JOY_SELECT)) {
            if (++page >= numPages) {
                return;
            }
            Sound_Play(SFX_MENU);
        }
        if (joyEdge & JOY_B) {
            return;
        }

        BG_Print(1, 1, 0, "%s license (pg %d/%d)", title, page + 1, numPages);
        BG_Print(1, 4, 0, "%s", pages[page]);
        BG_Display();
        Task_Yield();
    }
}

static void dispOpenMadoolaLicense(void) {
    const static char *openMadoolaLicense[] = {
        "Copyright @ 2023-2025\n\n"
        "Nathan Misner.\n\n\n\n"
        "OpenMadoola is free software:\n\n"
        "you can redistribute it and/or\n\n"
        "modify it under the terms of\n\n"
        "the GNU General Public License\n\n"
        "as published by the Free\n\n"
        "Software Foundation; either\n\n"
        "version 2 of the License, or\n\n"
        "(at your option) any later\n\n"
        "version.",

        "OpenMadoola is distributed in\n\n"
        "the hope that it will be\n\n"
        "useful, but WITHOUT ANY\n\n"
        "WARRANTY; without even the\n\n"
        "implied warranty of\n\n"
        "MERCHANTABILITY or FITNESS\n\n"
        "FOR A PARTICULAR PURPOSE. See\n\n"
        "the GNU General Public License for\n\n"
        "more details.\n\n\n\n"
        "You should have received a\n\n"
        "copy of the GNU General Public\n\n"
        "License along with",

        "OpenMadoola. If not, see\n\n"
        "https://www.gnu.org/licenses/"
    };
    displayLicense("OpenMadoola", openMadoolaLicense, ARRAY_LEN(openMadoolaLicense));
}

void dispLibcoLicense(void) {
    const static char *libcoLicense[] = {
        "Copyright byuu and the higan\n\n"
        "team\n\n\n\n"
        "Permission to use, copy,\n\n"
        "modify, and/or distribute this\n\n"
        "software for any purpose with\n\n"
        "or without fee is hereby\n\n"
        "granted, provided that the\n\n"
        "above copyright notice and\n\n"
        "this permission notice appear\n\n"
        "in all copies.\n\n\n\n"
        "THE SOFTWARE IS PROVIDED {AS",

        "IS} AND THE AUTHOR DISCLAIMS\n\n"
        "ALL WARRANTIES WITH REGARD TO\n\n"
        "THIS SOFTWARE INCLUDING ALL\n\n"
        "IMPLIED WARRANTIES OF\n\n"
        "MERCHANTABILITY AND FITNESS.\n\n"
        "IN NO EVENT SHALL THE AUTHOR\n\n"
        "BE LIABLE FOR ANY SPECIAL,\n\n"
        "DIRECT, INDIRECT, OR\n\n"
        "CONSEQUENTIAL DAMAGES OR ANY\n\n"
        "DAMAGES WHATSOEVER RESULTING\n\n"
        "FROM LOSS OF USE, DATA OR\n\n"
        "PROFITS, WHETHER IN AN ACTION\n\n"
        "OF CONTRACT, NEGLIGENCE OR",

        "TORTIOUS ACTION, ARISING OUT\n\n"
        "OF OR IN CONNECTION WITH THE\n\n"
        "USE OR PERFORMANCE OF THIS\n\n"
        "SOFTWARE.\n\n\n\n"
        "The above applies to all files\n\n"
        "in this project except\n\n"
        "valgrind.h which is licensed\n\n"
        "under a BSD-style license. See\n\n"
        "the license text and copyright\n\n"
        "notice contained within that\n\n"
        "file."
    };
    displayLicense("libco", libcoLicense, ARRAY_LEN(libcoLicense));
}

void dispNanotimeLicense(void) {
    const static char *nanotimeLicense[] = {
        "This is free and unencumbered\n\n"
        "software released into the\n\n"
        "public domain.\n\n\n\n"
        "Anyone is free to copy,\n\n"
        "modify, publish, use, compile,\n\n"
        "sell, or distribute this\n\n"
        "software, either in source\n\n"
        "code form or as a compiled\n\n"
        "binary, for any purpose,\n\n"
        "commercial or non-commercial,\n\n"
        "and by any means.",

        "In jurisdictions that\n\n"
        "recognize copyright laws, the\n\n"
        "author or authors of this\n\n"
        "software dedicate any and all\n\n"
        "copyright interest in the\n\n"
        "software to the public domain.\n\n"
        "We make this dedication for\n\n"
        "the benefit of the public at\n\n"
        "large and to the detriment of\n\n"
        "our heirs and successors. We\n\n"
        "intend this dedication to be\n\n"
        "an overt act of relinquishment\n\n"
        "in perpetuity of all present",

        "and future rights to this\n\n"
        "software under copyright law.\n\n\n\n"
        "THE SOFTWARE IS PROVIDED {AS\n\n"
        "IS}, WITHOUT WARRANTY OF ANY\n\n"
        "KIND, EXPRESS OR IMPLIED,\n\n"
        "INCLUDING BUT NOT LIMITED TO\n\n"
        "THE WARRANTIES OF\n\n"
        "MERCHANTABILITY, FITNESS FOR A\n\n"
        "PARTICULAR PURPOSE AND\n\n"
        "NONINFRINGEMENT. IN NO EVENT\n\n"
        "SHALL THE AUTHORS BE LIABLE\n\n"
        "FOR ANY CLAIM, DAMAGES OR",

        "OTHER LIABILITY, WHETHER IN AN\n\n"
        "ACTION OF CONTRACT, TORT OR\n\n"
        "OTHERWISE, ARISING FROM, OUT\n\n"
        "OF OR IN CONNECTION WITH THE\n\n"
        "SOFTWARE OR THE USE OR OTHER\n\n"
        "DEALINGS IN THE SOFTWARE.\n\n\n\n"
        "For more information, please\n\n"
        "refer to http://unlicense.org/"
    };
    displayLicense("nanotime", nanotimeLicense, ARRAY_LEN(nanotimeLicense));
}

void dispNesNtscLicense(void) {
    const static char *nesNtscLicense[] = {
        "Copyright @ 2006 Shay Green.\n\n\n\n"
        "This module is free software;\n\n"
        "you can redistribute it and/or\n\n"
        "modify it under the terms of\n\n"
        "the GNU Lesser General Public\n\n"
        "License as published by the\n\n"
        "Free Software Foundation;\n\n"
        "either version 2.1 of the\n\n"
        "License, or (at your option)\n\n"
        "any later version.\n\n\n\n"
        "This module is distributed",

        "in the hope that it will be\n\n"
        "useful, but WITHOUT ANY\n\n"
        "WARRANTY; without even the\n\n"
        "implied warranty of\n\n"
        "MERCHANTABILITY or FITNESS\n\n"
        "FOR A PARTICULAR PURPOSE. See\n\n"
        "the GNU Lesser General Public\n\n"
        "License for more details.\n\n\n\n"
        "You should have received a\n\n"
        "copy of the GNU Lesser General\n\n"
        "Public License along with this\n\n"
        "module; if not, write to the",

        "Free Software Foundation,\n\n"
        "Inc., 51 Franklin Street,\n\n"
        "Fifth Floor, Boston, MA\n\n"
        "02110-1301 USA"
    };
    displayLicense("nes_ntsc", nesNtscLicense, ARRAY_LEN(nesNtscLicense));
}

void dispNesSndEmuLicense(void) {
    const static char *nesSndEmuLicense[] = {
        "Copyright @ 2018-2025\n\n"
        "James Athey.\n\n"
        "Copyright @ 2003-2005\n\n"
        "Shay Green.\n\n\n\n"
        "This module is free software;\n\n"
        "you can redistribute it and/or\n\n"
        "modify it under the terms of\n\n"
        "the GNU Lesser General Public\n\n"
        "License as published by the\n\n"
        "Free Software Foundation;\n\n"
        "either version 2.1 of the\n\n"
        "License, or (at your option)",

        "any later version.\n\n\n\n"
        "This module is distributed\n\n"
        "in the hope that it will be\n\n"
        "useful, but WITHOUT ANY\n\n"
        "WARRANTY; without even the\n\n"
        "implied warranty of\n\n"
        "MERCHANTABILITY or FITNESS\n\n"
        "FOR A PARTICULAR PURPOSE. See\n\n"
        "the GNU Lesser General Public\n\n"
        "License for more details.",

        "You should have received a\n\n"
        "copy of the GNU Lesser General\n\n"
        "Public License along with this\n\n"
        "module; if not, write to the\n\n"
        "Free Software Foundation,\n\n"
        "Inc., 59 Temple Place, Suite\n\n"
        "330, Boston, MA 02111-1307 USA"
    };
    displayLicense("Nes_Snd_Emu", nesSndEmuLicense, ARRAY_LEN(nesSndEmuLicense));
}

#ifdef OM_PLATFORM_SDL2
void dispSDL2License(void) {
    const static char *sdl2License[] = {
        "Copyright @ 1997-2025 Sam\n\n"
        "Lantinga\n\n\n\n"
        "This software is provided\n\n"
        "'as-is', without any express\n\n"
        "or implied warranty. In no\n\n"
        "event will the authors be held\n\n"
        "liable for any damages arising\n\n"
        "from the use of this software.\n\n\n\n"
        "Permission is granted to\n\n"
        "anyone to use this software\n\n"
        "for any purpose, including",

        "commercial applications, and\n\n"
        "to alter it and redistribute\n\n"
        "it freely, subject to the\n\n"
        "following restrictions:\n\n\n\n"
        "1. The origin of this software\n\n"
        "must not be misrepresented;\n\n"
        "you must not claim that you\n\n"
        "wrote the original software.\n\n"
        "If you use this software in a\n\n"
        "product, an acknowledgment in\n\n"
        "the product documentation\n\n"
        "would be appreciated but is",

        "not required.\n\n"
        "2. Altered source versions\n\n"
        "must be plainly marked as\n\n"
        "such, and must not be\n\n"
        "misrepresented as being the\n\n"
        "original software.\n\n"
        "3. This notice may not be\n\n"
        "removed or altered from any\n\n"
        "source distribution."
    };
    displayLicense("SDL2", sdl2License, ARRAY_LEN(sdl2License));
}
#endif

#ifdef OM_PLATFORM_SDL3
void dispSDL3License(void) {
    const static char *sdl3License[] = {
        "Copyright @ 1997-2025 Sam\n\n"
        "Lantinga\n\n\n\n"
        "This software is provided\n\n"
        "'as-is', without any express\n\n"
        "or implied warranty. In no\n\n"
        "event will the authors be held\n\n"
        "liable for any damages arising\n\n"
        "from the use of this software.\n\n\n\n"
        "Permission is granted to\n\n"
        "anyone to use this software\n\n"
        "for any purpose, including",

        "commercial applications, and\n\n"
        "to alter it and redistribute\n\n"
        "it freely, subject to the\n\n"
        "following restrictions:\n\n\n\n"
        "1. The origin of this software\n\n"
        "must not be misrepresented;\n\n"
        "you must not claim that you\n\n"
        "wrote the original software.\n\n"
        "If you use this software in a\n\n"
        "product, an acknowledgment in\n\n"
        "the product documentation\n\n"
        "would be appreciated but is",

        "not required.\n\n"
        "2. Altered source versions\n\n"
        "must be plainly marked as\n\n"
        "such, and must not be\n\n"
        "misrepresented as being the\n\n"
        "original software.\n\n"
        "3. This notice may not be\n\n"
        "removed or altered from any\n\n"
        "source distribution."
    };
    displayLicense("SDL3", sdl3License, ARRAY_LEN(sdl3License));
}
#endif

void Licenses_Draw(void) {
    BG_Print(12, 2, 0, "Licenses");
}

void Licenses_Run(void) {
    static Uint8 palette[] = {
        0x0F, 0x36, 0x26, 0x16,
        0x0F, 0x11, 0x21, 0x31,
        0x0F, 0x12, 0x22, 0x32,
        0x0F, 0x13, 0x23, 0x33,
        0x0F, 0x36, 0x26, 0x16,
        0x0F, 0x11, 0x21, 0x31,
        0x0F, 0x12, 0x22, 0x32,
        0x0F, 0x13, 0x23, 0x33,
    };

    static MenuItem items[] = {
        MENU_LINK("OpenMadoola", dispOpenMadoolaLicense),
        MENU_LINK("libco", dispLibcoLicense),
        MENU_LINK("nanotime", dispNanotimeLicense),
        MENU_LINK("nes_ntsc", dispNesNtscLicense),
        MENU_LINK("Nes_Snd_Emu", dispNesSndEmuLicense),
    #ifdef OM_PLATFORM_SDL2
        MENU_LINK("SDL2", dispSDL2License),
    #endif
    #ifdef OM_PLATFORM_SDL3
        MENU_LINK("SDL3", dispSDL3License),
    #endif
        MENU_TASK("Back", MainMenu_Run),
    };

    BG_SetAllPalettes(palette);
    Sprite_SetAllPalettes(palette + 16);
    Menu_Run(10, 7, 2, items, ARRAY_LEN(items), Licenses_Draw);
}