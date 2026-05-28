#define _CRT_SECURE_NO_WARNINGS
#define REBUILD_PC
#define CTR_NATIVE
#define USE_EXTENDED_PRIM_POINTERS 0
#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if __GNUC__
#include <SDL2/SDL.h>
#if !defined(_WIN32)
#include <errno.h>
#include <sys/mman.h>
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif
#define _EnterCriticalSection(x)
#define EnterCriticalSection(x)
#define ExitCriticalSection()
#endif

#if defined(_WIN32)
#define CTR_MEM_COMMIT     0x00001000
#define CTR_MEM_RESERVE    0x00002000
#define CTR_PAGE_READWRITE 0x04
__declspec(dllimport) void *__stdcall VirtualAlloc(void *lpAddress, size_t dwSize, unsigned long flAllocationType, unsigned long flProtect);
__declspec(dllimport) unsigned long __stdcall GetLastError(void);
#endif

#include "psx/types.h"
#include "psx/libetc.h"
#include "psx/libgte.h"
#include "psx/libgpu.h"
#include "psx/libspu.h"
#include "psx/libcd.h"
#include "psx/libapi.h"
#include "psx/strings.h"
#include "psx/inline_c.h"
#include "PsyX/PsyX_public.h"
#include "PsyX/PsyX_globals.h"
#include "PsyX/PsyX_render.h"
#include "ctr_scratchpad.h"

static Uint32 startTick;
#define ResetRCnt(x) startTick = SDL_GetTicks();
#define GetRCnt(x)   ((SDL_GetTicks() - startTick) * 15720) / 1000

#define BUILD        926
#define u_long       u32

#ifndef __GNUC__
#define _Static_assert(x)
#endif
#define __attribute__(x)

#define RECT RECT16
typedef enum
{
	PAD_ID_MOUSE = 0x1,
	PAD_ID_NEGCON = 0x2,
	PAD_ID_IRQ10_GUN = 0x3,
	PAD_ID_DIGITAL = 0x4,
	PAD_ID_ANALOG_STICK = 0x5,
	PAD_ID_GUNCON = 0x6,
	PAD_ID_ANALOG = 0x7,
	PAD_ID_MULTITAP = 0x8,
	PAD_ID_JOGCON = 0xe,
	PAD_ID_CONFIG_MODE = 0xf,
	PAD_ID_NONE = 0xf
} PadTypeID;

#include "ctr_native.h"
#include "platform.h"

#ifndef CTR_NATIVE_MEMPACK_RETAIL_PRESSURE
#define CTR_NATIVE_MEMPACK_RETAIL_PRESSURE 0
#endif

#if CTR_NATIVE_MEMPACK_RETAIL_PRESSURE
// NOTE(aalhendi): Retail pressure mode exposes the NTSC-U 926 mempack window inside a 2 MiB backing store.
#define CTR_NATIVE_MEMPACK_BUFFER_SIZE  0x200000u
#define CTR_NATIVE_MEMPACK_START_OFFSET 0xba9f0u
#define CTR_NATIVE_MEMPACK_SIZE         0x144e10u
#else
#define CTR_NATIVE_MEMPACK_BUFFER_SIZE  (32u * 1024u * 1024u)
#define CTR_NATIVE_MEMPACK_START_OFFSET 0u
#define CTR_NATIVE_MEMPACK_SIZE         CTR_NATIVE_MEMPACK_BUFFER_SIZE
#endif

static char s_mempackMemory[CTR_NATIVE_MEMPACK_BUFFER_SIZE];
static struct PlatformMempackArena s_mempackArena;

#include "game_includes.h"

#include "game/zGlobal_RDATA.c"
#include "game/zGlobal_DATA.c"
#include "game/zGlobal_SDATA.c"

static int frameGap = 2000;
static int frameCount = 0;
static int oldTicks = 0;

void Platform_InitScratchpad(void)
{
#if defined(CTR_NATIVE)
	void *scratchpad = (void *)CTR_SCRATCHPAD_ADDR;
	size_t scratchpadSize = CTR_SCRATCHPAD_MAP_SIZE;

#if defined(_WIN32)
	void *mapped = VirtualAlloc(scratchpad, scratchpadSize, CTR_MEM_RESERVE | CTR_MEM_COMMIT, CTR_PAGE_READWRITE);
	if (mapped == NULL)
	{
		fprintf(stderr, "[CTR Native] Failed to map PS1 scratchpad at %p: GetLastError=%lu\n", scratchpad, GetLastError());
		abort();
	}
#elif defined(__GNUC__)
#ifdef MAP_FIXED_NOREPLACE
	int fixedFlag = MAP_FIXED_NOREPLACE;
#else
	int fixedFlag = MAP_FIXED;
#endif

	void *mapped = mmap(scratchpad, scratchpadSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | fixedFlag, -1, 0);
	if (mapped == MAP_FAILED)
	{
		fprintf(stderr, "[CTR Native] Failed to map PS1 scratchpad at %p: %s\n", scratchpad, strerror(errno));
		abort();
	}
#else
#error "Platform_InitScratchpad needs a fixed-address virtual-memory mapper for this platform"
#endif

	if (mapped != scratchpad)
	{
		fprintf(stderr, "[CTR Native] PS1 scratchpad mapped at %p, expected %p\n", mapped, scratchpad);
		abort();
	}

	memset(mapped, 0, scratchpadSize);
#endif
}

const struct PlatformMempackArena *Platform_InitMempackArena(void)
{
	memset(s_mempackMemory, 0, sizeof(s_mempackMemory));

	s_mempackArena.base = &s_mempackMemory[0];
	s_mempackArena.start = &s_mempackMemory[CTR_NATIVE_MEMPACK_START_OFFSET];
	s_mempackArena.endOfMemory = &s_mempackMemory[CTR_NATIVE_MEMPACK_BUFFER_SIZE];
	s_mempackArena.size = CTR_NATIVE_MEMPACK_SIZE;
	s_mempackArena.backingSize = CTR_NATIVE_MEMPACK_BUFFER_SIZE;

	// NOTE(aalhendi): PS1 OT links can pack these host pointers into 24-bit addresses.
	s_mempackArena.lowAddressValid =
	    ((u32)s_mempackArena.base < 0x01000000) && ((u32)s_mempackArena.start < 0x01000000) && ((u32)s_mempackArena.endOfMemory <= 0x01000000);

	return &s_mempackArena;
}

static void CalcFPS(void)
{
	if (frameCount++ != frameGap)
		return;

	frameCount = 0;
	int newTicks = SDL_GetTicks();
	int delta = newTicks - oldTicks;
	oldTicks = newTicks;

	printf("FPS: %d\n", (1000 * frameGap) / delta);
}

void PsyXKeyboardHandler(int key, char down)
{
	if (down == 0)
		key = 0;

	SubmitName_UseKeyboard(key);
}

#ifndef CC
#if __GNUC__
#if _WIN32
#ifndef __clang__
#define CC "MINGW-GCC"
#else
#define CC "MINGW-CLANG"
#endif
#else
#ifndef __clang__
#define CC "GCC"
#else
#define CC "CLANG"
#endif
#endif
#elif defined(_MSC_VER)
#define CC "MSVC"
#else
#define CC "Unknown"
#endif
#endif

int main(int argc, char *argv[])
{
	printf("[CTR Native] Starting...\n");
	fflush(stdout);

	char *sdlBasePath = SDL_GetBasePath();
	printf("[CTR Native] SDL base path: %s\n", sdlBasePath ? sdlBasePath : "(null)");
	fflush(stdout);

	char baseDir[1024];

	if (sdlBasePath)
	{
		strncpy(baseDir, sdlBasePath, sizeof(baseDir));
		SDL_free(sdlBasePath);
		char *sep = strrchr(baseDir, '\\');
		if (!sep)
			sep = strrchr(baseDir, '/');
		if (sep)
			*sep = '\0';
		sep = strrchr(baseDir, '\\');
		if (!sep)
			sep = strrchr(baseDir, '/');
		if (sep)
			*sep = '\0';
	}
	else
	{
		getcwd(baseDir, sizeof(baseDir));
	}

	printf("[CTR Native] Built with: " CC "\n");
	printf("[CTR Native] Base: %s\n", baseDir);
	fflush(stdout);

	chdir(baseDir);

#ifdef USE_16BY9
	printf("[CTR Native] Widescreen\n");
	Platform_Init("Crash Team Racing", 1280, 720);
#else
	printf("[CTR Native] 4:3\n");
	Platform_Init("Crash Team Racing", 800, 600);
#endif

	Platform_InitFilesystem("assets/ctr-u.bin");
	Platform_InitScratchpad();

	// // NOTE(aalhendi): CTR already throttles through the retail VSync/draw-sync
	// // path. Do not add a second SDL swap wait here, or gameplay runs as 64 ms
	// // physics frames and collision can tunnel through slopes.
	// PsyX_SetSwapInterval(0);
	// PsyX_EnableSwapInterval(0);

	// NOTE(aalhendi): PsyCross swap interval - combined with VSync() below, locks to 30fps
	PsyX_SetSwapInterval(2);
	PsyX_EnableSwapInterval(1);

	g_cfg_controllerToSlotMapping[0] = 0;

	g_dbg_gameDebugKeys = PsyXKeyboardHandler;

	int result = CTR_Main();

	Platform_Shutdown();
	return result;
}

void Platform_Init(const char *title, int width, int height)
{
	g_cfg_swapInterval = 1;
	PsyX_Initialise(title, width, height, 0);
}

void Platform_Shutdown(void)
{
}

void Platform_BeginFrame(void)
{
	PsyX_BeginScene();
}

// NOTE(aalhendi): Frame timing is handled by VSync() in the platform layer,
// matching PS1 hardware behavior. Platform_EndFrame only does buffer swap + FPS.
void Platform_EndFrame(void)
{
	PsyX_EndScene();
	CalcFPS();
}

void Platform_PresentVRAMDisplay(void)
{
	Platform_BeginFrame();
	GR_PresentVRAMDisplay();
	Platform_EndFrame();
}

int Platform_PollInput(void)
{
	return 1;
}

void Platform_InitFilesystem(const char *disc_image)
{
	PsyX_CDFS_Init(disc_image, 0, 0);
}

int NikoGetEnterKey(void)
{
	const u8 *kb = SDL_GetKeyboardState(NULL);
	return (kb && kb[SDL_SCANCODE_RETURN]) ? 1 : 0;
}

// NOTE(aalhendi): PS1 VSync emulation; game's primary frame throttle.
static Uint32 s_nextVBlank = 0;

int VSync(int mode)
{
	// NOTE(aalhendi): determine how many ms supposed to wait.
	// On PS1, mode <= 0 means wait for the next single VBlank (~16ms).
	// mode > 0 means explicitly wait for 'mode' number of VBlanks (mode * 16ms).
	Uint32 wait_ms = (mode <= 0) ? 16 : (16 * mode);

	Uint32 now = SDL_GetTicks();

	if (s_nextVBlank != 0 && now < s_nextVBlank)
	{
		SDL_Delay(s_nextVBlank - now);
		now = SDL_GetTicks();
	}

#ifdef CTR_INTERNAL
	const Uint32 OVERRUN_THRESHOLD_MS = 8;
	if (s_nextVBlank != 0 && now > s_nextVBlank + OVERRUN_THRESHOLD_MS)
	{
		// fprintf(stderr, "[CTR] VSync overrun by %d ms\n", now - s_nextVBlank);
	}
#endif

	s_nextVBlank = now + wait_ms;
	return (int)(now & 0x7FFF);
}
