# CTR Native

A native PC port of Crash Team Racing (PS1, 1999), built on top of the [CTR-ModSDK](https://github.com/CTR-tools/CTR-ModSDK) decompilation project.

## Philosophy

- **No byte budget.** Game source lives in `game/` as our own copies. Edit freely.
- **No PSX toolchain.** Targets Windows and Linux with SDL2 + PsyCross. No MIPS compiler needed.
- **Clean platform layer.** Game code calls through `platform.h`. SDL/PsyCross details stay in `ctr_native.c`.
- **No build system nonsense.** Just `build.bat` / `build.sh`.
- **Fully static build.** Single executable, zero dependencies. SDL2, OpenAL, and PsyCross are compiled from vendored source and linked statically.

## Directory Layout

```
ctr_native/
  ctr_native.c        Platform layer: SDL2 init, main(), frame loop
  ctr_native.h        Platform state structs
  platform.h          Platform API the game calls through
  game_includes.h     Ordered include chain for all game source files
  build.bat           Windows build (MinGW32)
  build.sh            Linux build
  README.md           This file
  game/               Our copies of all decompiled game source (943 files)
  include/            Project headers (structs, globals, declarations)
  externals/
    PsyCross/         PS1 hardware abstraction (GPU, GTE, SPU, CD)
    SDL/              SDL2 source (static build)
    openal-soft/      OpenAL source (static build)
```

## Prerequisites

### Windows

1. Install [MSYS2](https://www.msys2.org/)
2. In an MSYS2 terminal:
   ```
   pacman -S mingw-w64-i686-gcc mingw-w64-i686-tools-git
   ```
3. Add `C:\msys64\mingw32\bin` to your system PATH

That's it. SDL2 and OpenAL are compiled from vendored source — no separate install needed.

### Linux (Debian/Ubuntu)

```
sudo apt install gcc-multilib g++-multilib
sudo apt install libx11-dev libxext-dev libgl1-mesa-dev libasound2-dev libudev-dev libdbus-1-dev
```

## Building

```
build.bat            # Windows
chmod +x build.sh
./build.sh           # Linux
```

First build compiles SDL2, OpenAL, and PsyCross from source (~2-3 min). These are cached as static libraries in `build/` — subsequent builds only recompile `ctr_native.c`.

Output: `build/ctr_native.exe` (Windows) or `build/ctr_native` (Linux)

### Clean build

```
rm -rf build/        # Delete cached libraries
build.bat            # Rebuild everything
```

## Running

1. Create an `assets/` directory next to the source files
2. Extract the following from a CTR NTSC-U disc image (`ctr-u.bin`):
   - `BIGFILE.BIG`
   - `SOUNDS/KART.HWL`
   - `XA/ENG.XNF`
   - `XA/ENG/EXTRA/S00.XA` through `S05.XA`
   - `XA/ENG/GAME/S00.XA` through `S05.XA`
   - `XA/MUSIC/S00.XA` through `S05.XA`
3. Place `ctr-u.bin` itself in `assets/` as well
4. Run `build/ctr_native.exe`

Directory structure:
```
ctr_native/
  assets/
    ctr-u.bin
    BIGFILE.BIG
    SOUNDS/KART.HWL
    XA/
      ENG.XNF
      ENG/EXTRA/S00.XA ... S05.XA
      ENG/GAME/S00.XA ... S05.XA
      MUSIC/S00.XA ... S05.XA
```

## Architecture

```
ctr_native.c (platform layer)
  |
  +-- PsyCross (PS1 hardware abstraction: GPU, GTE, SPU, CD)
  |     |
  |     +-- SDL2 (window, input, timing)
  |     +-- OpenAL (audio)
  |
  +-- game_includes.h
        |
        +-- game/ (all decompiled game source)
              |
              +-- include/ (headers: structs, globals, declarations)
```

- `CTR_NATIVE` is defined for native host/platform-specific code
- The build uses 32-bit mode because the game stores 24-bit pointers in data structures. A fixed base address is not currently set (requires Clang/LLD or a custom linker script for GCC). This constraint will be removed in Phase 3.

## Roadmap

- Clean up `game/` copies strip byte budget hacks and route platform-specific code through `CTR_NATIVE`
- Replace PsyCross with SDL3 + custom renderer. Remove 32-bit constraint. Own the full stack.

## Credits

- [CTR-ModSDK](https://github.com/CTR-tools/CTR-ModSDK) — the decompilation project this is built on
- [PsyCross](https://github.com/OpenDriver2/PsyCross) — PS1 hardware abstraction layer
- [SDL2](https://github.com/libsdl-org/SDL) — cross-platform multimedia
- [openal-soft](https://github.com/kcat/openal-soft) — 3D audio API
- Crash Team Racing is a trademark of Sony Computer Entertainment / Naughty Dog
