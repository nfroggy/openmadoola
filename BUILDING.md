# Build Instructions

## SDL2

### UNIX-like platforms (GNU/Linux, Mac OS X, etc)

**Note:** Some fast-moving distros like Arch and Fedora have replaced their SDL2 packages with sdl2-compat, a compatibility wrapper that uses SDL3 as a backend to provide the SDL2 API. If your distro packages SDL3, make sure you build OpenMadoola for SDL3 instead of SDL2 to avoid any bugs introduced by the sdl2-compat wrapper.

Install GCC or Clang, make, cmake, and the SDL2 development libraries from your package manager.

Generate the build files:
```
cmake -B build -DACTIVE_PLATFORM=SDL2
```

Next, compile the executable:
```
cmake --build build
```

### Windows

Install [Visual Studio](https://visualstudio.microsoft.com/downloads/) and [cmake](https://cmake.org/download/).

Download the latest SDL2 VC development version (SDL2-devel-2.n.n-VC.zip) from [here](https://github.com/libsdl-org/SDL/releases). Unzip the file into libs\\SDL2, making sure that you go down a level so the top-level directories in the folder are docs\\ include\\ libs\\ etc.

Open the cmake gui, and click "Browse Source...". Browse to where you checked out the repo. Create a new folder inside your repo folder called "build". Click "Browse Build..." and browse to the build folder you just created. Click "Configure", and leave everything as default. Make sure the "ACTIVE_PLATFORM" dropdown is set to SDL2, then click "Generate".

Open the generated solution (inside the build folder) in Visual Studio. Right-click the "openmadoola" project, and click "Set as Startup Project". You should be able to build and/or debug openmadoola at this point.

Note that I don't check if the project builds with Visual Studio in between releases, so if you check out from master it's possible that the project won't build without changes. If you make a PR with the required changes, that would be appreciated.

## SDL3

### UNIX-like platforms (GNU/Linux, Mac OS X, etc)

Install GCC or Clang, make, cmake, and the SDL3 development libraries from your package manager.

Generate the build files:
```
cmake -B build -DACTIVE_PLATFORM=SDL3
```

Next, compile the executable:
```
cmake --build build
```

### Windows

Install [Visual Studio](https://visualstudio.microsoft.com/downloads/) and [cmake](https://cmake.org/download/).

Download the latest SDL3 VC development version (SDL3-devel-3.n.n-VC.zip) from [here](https://github.com/libsdl-org/SDL/releases). Unzip the file into libs\\SDL3, making sure that you go down a level so the top-level directories in the folder are cmake\\ include\\ lib\\ etc.

Open the cmake gui, and click "Browse Source...". Browse to where you checked out the repo. Create a new folder inside your repo folder called "build". Click "Browse Build..." and browse to the build folder you just created. Click "Configure", and leave everything as default. Set the "ACTIVE_PLATFORM" dropdown to SDL3, then click "Generate".

Open the generated solution (inside the build folder) in Visual Studio. Right-click the "openmadoola" project, and click "Set as Startup Project". You should be able to build and/or debug openmadoola at this point.

Note that I don't check if the project builds with Visual Studio in between releases, so if you check out from master it's possible that the project won't build without changes. If you make a PR with the required changes, that would be appreciated.
