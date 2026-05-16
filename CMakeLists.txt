cmake_minimum_required(VERSION 3.20)
project(CTR-Native C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_C_COMPILER_TARGET MATCHES "i.86" AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "i.86")
    set(CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "-m32 ${CMAKE_EXE_LINKER_FLAGS}")
endif()

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ ${CMAKE_EXE_LINKER_FLAGS}")

set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)

set(GAME_SOURCES
    ctr_native.c
)

set(GAME_INCLUDES
    ${CMAKE_SOURCE_DIR}/include
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -w -Wno-error -Wno-implicit-function-declaration -Wno-int-conversion -Wno-incompatible-pointer-types -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse -fpermissive -w -Wno-error -Wno-narrowing -Wno-incompatible-pointer-types -g")

# SDL2
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(externals/SDL)
target_include_directories(SDL2-static PUBLIC externals/SDL/include)

# OpenAL
set(LIBTYPE STATIC CACHE STRING "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
set(CMAKE_CXX_STANDARD 17 CACHE STRING "" FORCE)
add_subdirectory(externals/openal-soft)
set_target_properties(OpenAL PROPERTIES CXX_STANDARD 17)
target_include_directories(OpenAL PUBLIC externals/openal-soft/include)

# PsyCross
file(GLOB_RECURSE PSYCROSS_C externals/PsyCross/*.c externals/PsyCross/*.C)
file(GLOB_RECURSE PSYCROSS_CPP externals/PsyCross/*.cpp)
set_source_files_properties(${PSYCROSS_C} PROPERTIES LANGUAGE C)
set_source_files_properties(${PSYCROSS_CPP} PROPERTIES LANGUAGE CXX)
add_library(psycross STATIC ${PSYCROSS_C} ${PSYCROSS_CPP})
target_include_directories(psycross PUBLIC externals/PsyCross/include externals/PsyCross)
target_compile_definitions(psycross PRIVATE USE_EXTENDED_PRIM_POINTERS=0)
target_include_directories(psycross PRIVATE externals/SDL/include externals/openal-soft/include)
target_link_libraries(psycross SDL2-static OpenAL)

# TODO(aalhendi): handle symbol conflict in a better way
if(WIN32)
    set(_P "_")
else()
    set(_P "")
endif()
add_custom_command(TARGET psycross POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
        --redefine-sym ${_P}alGenEffects=${_P}_psx_alGenEffects
        --redefine-sym ${_P}alEffectf=${_P}_psx_alEffectf
        --redefine-sym ${_P}alEffecti=${_P}_psx_alEffecti
        --redefine-sym ${_P}alGenAuxiliaryEffectSlots=${_P}_psx_alGenAuxiliaryEffectSlots
        --redefine-sym ${_P}alDeleteAuxiliaryEffectSlots=${_P}_psx_alDeleteAuxiliaryEffectSlots
        --redefine-sym ${_P}alDeleteEffects=${_P}_psx_alDeleteEffects
        --redefine-sym ${_P}alAuxiliaryEffectSloti=${_P}_psx_alAuxiliaryEffectSloti
        $<TARGET_FILE:psycross> $<TARGET_FILE:psycross>.fixed
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:psycross>.fixed $<TARGET_FILE:psycross>
    COMMENT "Renaming conflicting OpenAL EFX symbols in psycross"
)

# CTR Native
add_executable(ctr_native ${GAME_SOURCES})
target_include_directories(ctr_native PRIVATE ${GAME_INCLUDES})
target_compile_definitions(ctr_native PRIVATE
    REBUILD_PC
    CTR_NATIVE
    REBUILD_PS1
    USE_EXTENDED_PRIM_POINTERS=0
    USE_PCDRV
    USE_MOREPRIM
    BUILD=926
)
target_compile_options(ctr_native PRIVATE -O2)
target_link_libraries(ctr_native OpenAL psycross SDL2-static)
if(WIN32)
    target_link_options(ctr_native PRIVATE -static -static-libgcc -static-libstdc++ -Wl,--image-base,0x400000 -Wl,--disable-dynamicbase -Wl,--disable-reloc-section)
else()
    target_link_options(ctr_native PRIVATE -static-libgcc -static-libstdc++ -Wl,--image-base,0x400000 -no-pie -Wl,--allow-multiple-definition)
endif()
