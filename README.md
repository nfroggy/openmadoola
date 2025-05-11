# OpenMadoola

Free software (GPL v2.0 or later) reimplementation of "The Wing of Madoola" with bug fixes and optional gameplay enhancements. OpenMadoola was developed so people who own "The Wing of Madoola" on cartridge or on Steam can have more fun with their game. OpenMadoola is not a standalone game and will not function without externally supplied game data. This project is not licensed or endorsed by Sunsoft.

![OpenMadoola screenshot](screenshot.png)

## Downloading the game (pre-built)

If you run GNU/Linux, you can [download the latest release from Flathub](https://flathub.org/apps/com.infochunk.OpenMadoola) if you don't feel like building it from source. The Flatpak version of the game has a couple minor things to note due to sandboxing, see [here](FLATPAK.md) for more information.

If you run Windows, you can download the latest release from the Releases page of this repo.

If you run Mac OS X, you're going to have to build the game from source. I don't feel like paying Apple $100/year for a signing certificate.

## [Build instructions](BUILDING.md)

## Usage

In order to function, OpenMadoola requires the font.bin, 2c04.pal, and nes.pal data files as well as the demo and mml directories from this repository. These are included with the pre-built Windows binaries on the Releases page. OpenMadoola also requires a Wing of Madoola ROM image. The ROM image can either be a standard NES ROM file named "madoola.nes" or the "sharedassets0.assets" file from [Sunsoft is Back! Retro Game Selection](https://store.steampowered.com/app/2821290/SUNSOFT_is_Back/) on Steam. OpenMadoola reads all the graphics and level data from the ROM image, and will refuse to start if it is not present.

If you don't have an original cartridge or don't want to deal with dumping it, I would recommend buying the collection on Steam rather than downloading a ROM off the internet. It's only $13 and has a nice interface with high-quality scans of the manual as well as some concept art. It also shows Sunsoft that there's interest in their legacy IP. To retrieve the "sharedassets0.assets" file, go to your Steam library, right click the Sunsoft collection, and click "Manage->Browse local files". It will be in the "SUNSOFT is Back! Retro Game Selection_Data" directory.

### UNIX-like platforms (GNU/Linux, Mac OS X, etc)

OpenMadoola will attempt to read the ROM image and data files from these directories:

1. `$XDG_DATA_HOME/openmadoola`
2. `~/.openmadoola`
3. `/app/share/openmadoola`
4. `/usr/local/share/openmadoola`
5. `/usr/share/openmadoola`
6. Current working directory

It will also write configuration and save data to `$XDG_DATA_HOME/openmadoola` if it exists, otherwise `~/.openmadoola`.

### Windows

OpenMadoola will attempt to read the ROM image and data files from the current working directory (typically the directory the executable is in). If it doesn't see a ROM image in the current working directory, it will also try reading from the default Steam install directory (C:\Program Files (x86)\Steam\steamapps\common\SUNSOFT is Back! レトロゲームセレクション). OpenMadoola will write configuration and save data to the current working directory.

### Default controls

Keyboard | Gamepad (xbox layout) | Equivalent NES button
-------- | --------------------- | ---------------------
Up | D-Pad Up | D-Pad Up
Down | D-Pad Down | D-Pad Down
Left | D-Pad Left | D-Pad Left
Right | D-Pad Right | D-Pad Right
X | X | B
Z | A | A
Right Shift | Back | Select
Enter | Start | Start

You can change the controls in the options menu.

**Pressing A+B+Select+Start while in-game will exit to the title screen.**

### Game types

Also in the options menu, you'll find a "game type" setting. The "Original" game type tries to simulate the original NES game but with important bugfixes. The "Plus" game type (the default) includes additional fixes for more minor bugs as well as tweaks that I think improve the game. The "Arcade" game type is a recreation of the VS. System port.

#### Changes for all game types
- Game runs without NES hardware limitations (no slowdown, no sprite flicker, no garbage on the edge of the screen).
- Game progress is saved to disk.
- The status/stage screens can be skipped by pressing the Start button.
- Fixed a bug where collecting the Wing of Madoola and then entering a door would make Darutos not spawn.
- Fixed a bug where killing Darutos and then dying to one of his fireballs would let you skip fighting him after continuing.
- Fixed a bug where sound effects would sometimes get stuck on, glitch out, etc.
- Fixed a bug where pausing and then unpausing on a castle level would cause the noise channel to drop out of the music.
- Fixed a bug where the mirroring on the fireball that some enemies throw wasn't initialized.

#### Plus-exclusive changes
- Added the opening/ending text from the arcade version of the game.
- Added a second emulated sound chip just for music (meaning that channels won't cut out to play sound effects).
- Music doesn't restart when warping within the same stage.
- Music doesn't restart when pausing/unpausing the game.
- The arcade version of the boss music is used instead of the console version.
- The selected weapon is visible during gameplay, and you can change weapons without pausing.
- Darutos gets drawn even when he's only partially visible, making him easier to see.
- You can kill multiple enemies at once when they're on top of each other (makes dealing with Nyurus much less annoying).
- Fixed a bug where the orb you get at the end of a level wasn't animated correctly.
- Fixed a bug where Nipata would get stuck if it spawned in a narrow gap.

#### Arcade-exclusive changes
For a list of the all the changes I'm aware of between the console and arcade versions, see [here](https://www.infochunk.com/madoola/arcade.html). Here's a list of the changes between the real arcade version and the OpenMadoola version:
- The Plus changes (see above) are included.
- Anything related to inserting coins isn't here. The continue system is the same as the other game types.
- The "Press Start" text on the title screen blinks slower than the original because I think it looks nicer.
- The "Arcade difficulty" option in the settings controls the game's difficulty like this:
   - **Normal**: Lucia's health gets decremented by 1 every 32 frames.
   - **Hard**: There's no HP timer, but Lucia's MP only refills up to 1000 when she dies or between levels. You can refill her MP by scoring a multiple of 10,000 points. This mode matches [this](https://www.youtube.com/watch?v=C9T8ad12eO8) gameplay video.
   - **Crazy**: This is a combination of the HP timer from Normal difficulty and the MP refill mechanic from Hard difficulty. This mode matches [this](https://www.youtube.com/watch?v=FKKC1KHHi4Q) gameplay video.
- There's enemy collision in the attract mode. I added a couple other stages besides Stage 1 to the attract mode to add some variety.
- Lucia starts with 3 lives instead of 2. This may have originally been a DIP switch though.
- Because OpenMadoola has start/select buttons, none of the control changes have been ported over.
- I didn't add the Stage 4 change because it seems like a mistake.
- I didn't add the camera change because I like the camera from the console version (which centers Lucia when she's on solid ground) more than the arcade version (which only moves up/down when Lucia is at the edge of the screen).
- The high score entry screen is made up because there's no public footage of it.
- There's no option for increased enemy damage.
- The game over screen doesn't play after you beat the game (kind of a buzz-kill IMO).
- Lucia can only use the fountain once per life, per stage. Otherwise you can get stay in a stage indefinitely and kill enemies over and over, which breaks the scoring system. I don't think any of the footage of the game shows Lucia using the fountain multiple times in one life, so I don't know if the real game has the same restriction.
- All the default high scores are now 10x lower. In my opinion, the default high scores should be relatively easy to beat.

## Used Software

### Blip_Buffer http://www.slack.net/~ant/libs/audio.html#Blip_Buffer
```
Copyright (C) 2003-2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details. You should have received a copy of the GNU Lesser General
Public License along with this module; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
```

### libco https://github.com/higan-emu/libco
```
ISC License (ISC)

Copyright byuu and the higan team

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above copyright
notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.

The above applies to all files in this project except valgrind.h which is
licensed under a BSD-style license. See the license text and copyright notice
contained within that file.
```

### nanotime https://github.com/nightmareci/nanotime
```
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute
this software, either in source code form or as a compiled binary, for any
purpose, commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of
this software dedicate any and all copyright interest in the software to the
public domain. We make this dedication for the benefit of the public at
large and to the detriment of our heirs and successors. We intend this
dedication to be an overt act of relinquishment in perpetuity of all present
and future rights to this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
```

### nes_ntsc http://www.slack.net/~ant/libs/ntsc.html#nes_ntsc
```
Copyright (C) 2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
```

### Nes_Snd_Emu http://www.slack.net/~ant/libs/audio.html#Nes_Snd_Emu
```
Copyright (C) 2003-2005 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details. You should have received a copy of the GNU Lesser General
Public License along with this module; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
```

### sdl2 http://libsdl.org/
```
Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>
  
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:
  
1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required. 
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
```

### sdl3 http://libsdl.org/
```
Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>
  
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:
  
1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required. 
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
```
