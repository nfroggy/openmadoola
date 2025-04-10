cmake_minimum_required(VERSION 3.18)

project(openmadoola LANGUAGES C CXX)

set(PLATFORM_LIST SDL2 SDL3)
set(ACTIVE_PLATFORM SDL2 CACHE STRING "Default platform is SDL2")
set_property(CACHE ACTIVE_PLATFORM PROPERTY STRINGS ${PLATFORM_LIST})

option(SANITIZE "Compile with asan/ubsan (gcc/clang only)" OFF)

if(MSVC)
    add_executable(openmadoola WIN32 dist/openmadoola.rc)
else()
    add_executable(openmadoola WIN32)
endif()

target_sources(openmadoola PRIVATE
    # game code
    "src/main.c"
    "src/alloc.c"
    "src/bg.c"
    "src/buffer.c"
    "src/camera.c"
    "src/collision.c"
    "src/db.c"
    "src/demo.c"
    "src/ending.c"
    "src/enemy.c"
    "src/file.c"
    "src/game.c"
    "src/graphics.c"
    "src/highscore.c"
    "src/hud.c"
    "src/input.c"
    "src/joy.c"
    "src/licenses.c"
    "src/mainmenu.c"
    "src/map.c"
    "src/menu.c"
    "src/mml.c"
    "src/object.c"
    "src/options.c"
    "src/palette.c"
    "src/rng.c"
    "src/rom.c"
    "src/save.c"
    "src/screen.c"
    "src/sound.c"
    "src/soundtest.c"
    "src/sprite.c"
    "src/system.c"
    "src/task.c"
    "src/textscroll.c"
    "src/title.c"
    "src/util.c"
    "src/weapon.c"

    # object code
    "src/objects/biforce.c"
    "src/objects/bospido.c"
    "src/objects/boundball.c"
    "src/objects/bunyon.c"
    "src/objects/darutos.c"
    "src/objects/dopipu.c"
    "src/objects/explosion.c"
    "src/objects/eyemon.c"
    "src/objects/fire.c"
    "src/objects/fireball.c"
    "src/objects/flamesword.c"
    "src/objects/fountain.c"
    "src/objects/gaguzul.c"
    "src/objects/hopegg.c"
    "src/objects/item.c"
    "src/objects/joyraima.c"
    "src/objects/kikura.c"
    "src/objects/lucia.c"
    "src/objects/magicbomb.c"
    "src/objects/mantleskull.c"
    "src/objects/nigito.c"
    "src/objects/nipata.c"
    "src/objects/nishiga.c"
    "src/objects/nomaji.c"
    "src/objects/nyuru.c"
    "src/objects/peraskull.c"
    "src/objects/shieldball.c"
    "src/objects/smasher.c"
    "src/objects/spajyan.c"
    "src/objects/suneisa.c"
    "src/objects/sword.c"
    "src/objects/weaponfire.c"
    "src/objects/wingofmadoola.c"
    "src/objects/yokkochan.c"
    "src/objects/zadofly.c"

    # vendored libraries
    "libs/blargg_apu/Blip_Buffer.cpp"
    "libs/blargg_apu/Nes_Apu.cpp"
    "libs/blargg_apu/Nes_Oscs.cpp"
    "libs/blargg_apu/blargg_apu.cpp"

    "libs/libco/libco.c"

    "libs/nanotime/nanotime.c"

    "libs/nes_ntsc/nes_ntsc.c"

    # game code headers
    "src/alloc.h"
    "src/bg.h"
    "src/buffer.h"
    "src/camera.h"
    "src/collision.h"
    "src/constants.h"
    "src/db.h"
    "src/demo.h"
    "src/ending.h"
    "src/enemy.h"
    "src/file.h"
    "src/game.h"
    "src/graphics.h"
    "src/highscore.h"
    "src/hud.h"
    "src/input.h"
    "src/joy.h"
    "src/licenses.h"
    "src/mainmenu.h"
    "src/map.h"
    "src/menu.h"
    "src/mml.h"
    "src/object.h"
    "src/options.h"
    "src/palette.h"
    "src/platform.h"
    "src/rng.h"
    "src/rom.h"
    "src/save.h"
    "src/screen.h"
    "src/sound.h"
    "src/soundtest.h"
    "src/sprite.h"
    "src/system.h"
    "src/task.h"
    "src/textscroll.h"
    "src/title.h"
    "src/util.h"
    "src/weapon.h"
    
    # object code headers
    "src/objects/biforce.h"
    "src/objects/bospido.h"
    "src/objects/boundball.h"
    "src/objects/bunyon.h"
    "src/objects/darutos.h"
    "src/objects/dopipu.h"
    "src/objects/explosion.h"
    "src/objects/eyemon.h"
    "src/objects/fire.h"
    "src/objects/fireball.h"
    "src/objects/flamesword.h"
    "src/objects/fountain.h"
    "src/objects/gaguzul.h"
    "src/objects/hopegg.h"
    "src/objects/item.h"
    "src/objects/joyraima.h"
    "src/objects/kikura.h"
    "src/objects/lucia.h"
    "src/objects/magicbomb.h"
    "src/objects/mantleskull.h"
    "src/objects/nigito.h"
    "src/objects/nipata.h"
    "src/objects/nishiga.h"
    "src/objects/nomaji.h"
    "src/objects/nyuru.h"
    "src/objects/peraskull.h"
    "src/objects/shieldball.h"
    "src/objects/smasher.h"
    "src/objects/spajyan.h"
    "src/objects/sword.h"
    "src/objects/weaponfire.h"
    "src/objects/wingofmadoola.h"
    "src/objects/yokkochan.h"
    "src/objects/zadofly.h"
    
    # vendored library headers
    "libs/blargg_apu/blargg_apu.h"
    "libs/blargg_apu/blargg_common.h"
    "libs/blargg_apu/blargg_source.h"
    "libs/blargg_apu/Blip_Buffer.h"
    "libs/blargg_apu/Nes_Apu.h"
    "libs/blargg_apu/Nes_Oscs.h"

    "libs/libco/libco.h"

    "libs/nanotime/nanotime.h"

    "libs/nes_ntsc/nes_ntsc.h"
    "libs/nes_ntsc/nes_ntsc_config.h"
    "libs/nes_ntsc/nes_ntsc_impl.h"
)

target_include_directories(openmadoola PRIVATE
    "src"
    "src/objects"
    "libs/blargg_apu"
    "libs/libco"
    "libs/nanotime"
    "libs/nes_ntsc"
)

# platform stuff
if(ACTIVE_PLATFORM STREQUAL SDL2)
    # use vendored sdl2 lib on msvc
    if(MSVC)
        set(SDL2_INCLUDE_DIRS "libs/SDL2/include")
        set(SDL2_LINK_LIBRARIES "${PROJECT_SOURCE_DIR}/libs/SDL2/lib/x64/SDL2.lib;${PROJECT_SOURCE_DIR}/libs/SDL2/lib/x64/SDL2main.lib")
        set(MSVC_PLATFORM_DLL_PATH ${PROJECT_SOURCE_DIR}/libs/SDL2/lib/x64)
        # otherwise use system sdl2
    else()
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(SDL2 REQUIRED sdl2)
        target_compile_options(openmadoola PRIVATE ${SDL2_CFLAGS_OTHER})
    endif()
    # set up platform vars
    target_sources(openmadoola PRIVATE "src/platform_sdl2.c")
    target_include_directories(openmadoola PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(openmadoola PRIVATE ${SDL2_LINK_LIBRARIES})
    target_compile_definitions(openmadoola PRIVATE OM_PLATFORM_SDL2)
elseif(ACTIVE_PLATFORM STREQUAL SDL3)
    # use vendored sdl3 lib on msvc
    if(MSVC)
        target_include_directories(openmadoola PRIVATE "libs/SDL3/include")
        target_link_libraries(openmadoola PRIVATE "${PROJECT_SOURCE_DIR}/libs/SDL3/lib/x64/SDL3.lib")
        set(MSVC_PLATFORM_DLL_PATH ${PROJECT_SOURCE_DIR}/libs/SDL3/lib/x64)
        # otherwise use system sdl3
    else()
        find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)
        target_link_libraries(openmadoola PRIVATE SDL3::SDL3)
    endif()
    target_sources(openmadoola PRIVATE "src/platform_sdl3.c")
    target_compile_definitions(openmadoola PRIVATE OM_PLATFORM_SDL3)
endif()

# make visual studio folders work correctly
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

# compiler warnings
if(MSVC)
    target_compile_options(openmadoola PRIVATE /W3)
else()
    target_compile_options(openmadoola PRIVATE -Wall -Wvla -Wformat=2)
    if(SANITIZE)
        target_compile_options(openmadoola PRIVATE -fsanitize=address,undefined)
        target_link_options(openmadoola PRIVATE -fsanitize=address,undefined)
    endif()
endif()

# language versions
set_target_properties(openmadoola PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# endianness
include(TestBigEndian)
test_big_endian(endian)
if(endian)
    target_compile_definitions(openmadoola PRIVATE OM_BIG_ENDIAN)
endif()
    
# work around msvc nonsense
if(MSVC)
    # disable warnings when not using "_s" stdlib functions
    target_compile_definitions(openmadoola PRIVATE _CRT_SECURE_NO_WARNINGS)
    # read source files as utf-8
    target_compile_options(openmadoola PRIVATE /utf-8)
    # use "main" instead of "WinMain"
    set_target_properties(openmadoola PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
    # set the debugger working directory
    set_target_properties(openmadoola PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    # make sure the debugger can find the sdl2 dll file
    set(OM_PATH "PATH=%PATH%" ${MSVC_PLATFORM_DLL_PATH})
    set_target_properties(openmadoola PROPERTIES VS_DEBUGGER_ENVIRONMENT "${OM_PATH}")
    # statically link the runtime library in case the user doesn't have msvc redistributable installed
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
