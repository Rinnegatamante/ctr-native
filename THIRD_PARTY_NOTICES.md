# Third-Party Notices

This project vendors and modifies third-party software. Keep this file with
source and binary distributions of CTR Native.

## PsyCross / Psy-X

Path: `externals/PsyCross`

PsyCross provides the Psy-Q-compatible PS1 hardware abstraction layer used by
CTR Native, including compatible GPU, GTE, SPU, CD, and controller library
interfaces. CTR Native also ports some host-side behavior into its own native
platform layer while preserving Psy-Q-shaped APIs.

CTR Native contains project-owned derivatives of these PsyCross source files:

- `include/platform/native_gpu.h` from `externals/PsyCross/src/gpu/PsyX_GPU.h`
- `include/platform/native_psyx_shell.h` from
  `externals/PsyCross/src/PsyX_main.h`
- `include/platform/native_renderer_backend.h` and
  `include/platform/native_renderer_types.h` from
  `externals/PsyCross/include/PsyX/PsyX_render.h`
- `platform/native_gpu.c` from `externals/PsyCross/src/gpu/PsyX_GPU.cpp`
- `platform/native_libetc.c` from `externals/PsyCross/src/psx/LIBETC.C`
- `platform/native_libgpu.c` from `externals/PsyCross/src/psx/LIBGPU.C`
- `platform/native_psyx_shell.c` from
  `externals/PsyCross/src/PsyX_main.cpp`
- `platform/native_renderer_backend.c` from
  `externals/PsyCross/src/render/PsyX_render.cpp`
- `include/platform/native_glad.h` from
  `externals/PsyCross/include/PsyX/common/glad.h`
- `platform/native_glad.c` from `externals/PsyCross/src/render/glad.c`

License: MIT

Copyright (c) 2020 REDRIVER2 Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## SDL2

Path: `externals/SDL`

SDL2 provides cross-platform host windowing, input, timing, and audio device
support for CTR Native.

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
