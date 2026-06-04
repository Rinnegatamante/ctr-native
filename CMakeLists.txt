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
    platform/native_audio.c
    platform/native_cd.c
    platform/native_gpu.c
    platform/native_glad.c
    platform/native_input.c
    platform/native_libetc.c
    platform/native_libgpu.c
    platform/native_libpad.c
    platform/native_libspu.c
    platform/native_memcard.c
    platform/native_psyx_shell.c
    platform/native_renderer.c
    platform/native_renderer_backend.c
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

# PsyCross
# NOTE(aalhendi): limit native target to PsyCross units that ctr_native links atm.
set(PSYCROSS_C
    externals/PsyCross/src/psx/INLINE_C.C
    externals/PsyCross/src/psx/LIBAPI.C
    externals/PsyCross/src/psx/LIBGTE.C
)
set(PSYCROSS_CPP
    externals/PsyCross/src/gte/PsyX_GTE.cpp
)
set_source_files_properties(${PSYCROSS_C} PROPERTIES LANGUAGE C)
set_source_files_properties(${PSYCROSS_CPP} PROPERTIES LANGUAGE CXX)
add_library(psycross STATIC ${PSYCROSS_C} ${PSYCROSS_CPP})
target_include_directories(psycross PUBLIC externals/PsyCross/include externals/PsyCross)
target_compile_definitions(psycross PRIVATE USE_EXTENDED_PRIM_POINTERS=0)
target_include_directories(psycross PRIVATE externals/SDL/include)
target_link_libraries(psycross SDL2-static)

# CTR Native
add_executable(ctr_native ${GAME_SOURCES})
target_include_directories(ctr_native PRIVATE ${GAME_INCLUDES})
target_compile_definitions(ctr_native PRIVATE
    CTR_NATIVE
    USE_EXTENDED_PRIM_POINTERS=0
    BUILD=926
    CTR_INTERNAL
)
target_compile_options(ctr_native PRIVATE -O2)
target_link_libraries(ctr_native psycross SDL2-static)
if(WIN32)
    target_link_options(ctr_native PRIVATE -static -static-libgcc -static-libstdc++ -Wl,--image-base,0x400000 -Wl,--disable-dynamicbase -Wl,--disable-reloc-section)
else()
    target_link_options(ctr_native PRIVATE -static-libgcc -static-libstdc++ -Wl,--image-base,0x400000 -no-pie -Wl,--allow-multiple-definition)
endif()
