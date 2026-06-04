#define _CRT_SECURE_NO_WARNINGS
#define CTR_NATIVE
#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if __GNUC__
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
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
#include "ctr_scratchpad.h"
#include "platform/native_glad.h"
#include "platform/native_gpu.h"
#include "platform/native_input.h"
#include "platform/native_log.h"
#include "platform/native_renderer.h"

static int s_hostAltKeyState = 0;

#define BUILD  926
#define u_long u32

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
#include "platform/native_audio.h"

#ifndef CTR_NATIVE_MEMPACK_RETAIL_PRESSURE
#define CTR_NATIVE_MEMPACK_RETAIL_PRESSURE 1
#endif

// TODO(aalhendi): Re-audit LOAD_ReadFile_ex, LOAD_DramFileCallback, LEV/PTR
// callbacks, and hub swapping before removing the expanded arena escape hatch.
#if CTR_NATIVE_MEMPACK_RETAIL_PRESSURE
// NOTE(aalhendi): Retail pressure mode exposes the NTSC-U 926 mempack window inside a 2 MiB backing store.
#define CTR_NATIVE_MEMPACK_BUFFER_SIZE  0x200000u
#define CTR_NATIVE_MEMPACK_START_OFFSET 0xba9f0u
#define CTR_NATIVE_MEMPACK_SIZE         0x144e10u
#else
#define CTR_NATIVE_MEMPACK_BUFFER_SIZE  (8u * 1024u * 1024u)
#define CTR_NATIVE_MEMPACK_START_OFFSET 0u
#define CTR_NATIVE_MEMPACK_SIZE         CTR_NATIVE_MEMPACK_BUFFER_SIZE
#endif

static char s_mempackMemory[CTR_NATIVE_MEMPACK_BUFFER_SIZE];
static struct PlatformMempackArena s_mempackArena;

SDL_Window *g_window = NULL;
int g_dbg_polygonSelected = 0;

extern int g_cfg_bilinearFiltering;
extern int g_dbg_emulatorPaused;
extern int g_dbg_texturelessMode;
extern int g_dbg_wireframeMode;
extern int g_windowHeight;
extern int g_windowWidth;

static int s_platformInitialized = 0;
static int s_platformBeginScene = 0;

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

	// NOTE(aalhendi): Native still uses PS1-shaped OT links for many render
	// paths. MEMPACK must stay below 0x01000000 so CtrGpu_PrimToOTLink24 can
	// pack linked primitive pointers without losing address bits.
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

static void Platform_GetWindowName(const char *appName, char *buffer, size_t bufferSize)
{
#ifdef CTR_INTERNAL
	snprintf(buffer, bufferSize, "%s | Internal", appName);
#else
	snprintf(buffer, bufferSize, "%s", appName);
#endif
}

static void Platform_HandleWindowResize(int width, int height)
{
	g_windowWidth = width;
	g_windowHeight = height;
	NativeRenderer_ResetDevice();
}

static void Platform_HandleFullscreenToggle(void)
{
	int fullscreen = (SDL_GetWindowFlags(g_window) & SDL_WINDOW_FULLSCREEN) != 0;

	SDL_SetWindowFullscreen(g_window, fullscreen == 0);
	SDL_GetWindowSize(g_window, &g_windowWidth, &g_windowHeight);
	NativeRenderer_ResetDevice();
}

#if defined(CTR_INTERNAL)
static void Platform_TakeScreenshot(void)
{
	u8 *pixels = (u8 *)malloc(g_windowWidth * g_windowHeight * 4);

	glReadPixels(0, 0, g_windowWidth, g_windowHeight, GL_BGRA, GL_UNSIGNED_BYTE, pixels);

	SDL_Surface *surface = SDL_CreateSurfaceFrom(g_windowWidth, g_windowHeight, SDL_PIXELFORMAT_BGRA8888, pixels, g_windowWidth * 4);

	SDL_SaveBMP(surface, "SCREENSHOT.BMP");
	SDL_DestroySurface(surface);

	free(pixels);
}
#endif

static void Platform_HandleKey(int key, char down)
{
	if (down == 0)
		SubmitName_UseKeyboard(0);
	else
		SubmitName_UseKeyboard(key);

#ifdef CTR_INTERNAL
	if (!down)
	{
		switch (key)
		{
		case SDL_SCANCODE_F1:
			g_dbg_wireframeMode ^= 1;
			Platform_LogWarn("[CTR Native] wireframe mode: %d\n", g_dbg_wireframeMode);
			break;

		case SDL_SCANCODE_F2:
			g_dbg_texturelessMode ^= 1;
			Platform_LogWarn("[CTR Native] textureless mode: %d\n", g_dbg_texturelessMode);
			break;
		case SDL_SCANCODE_UP:
		case SDL_SCANCODE_DOWN:
			if (g_dbg_emulatorPaused)
			{
				g_dbg_polygonSelected += (key == SDL_SCANCODE_UP) ? 3 : -3;
			}
			break;
		case SDL_SCANCODE_F10:
			Platform_LogWarn("[CTR Native] saving VRAM.TGA\n");
			NativeRenderer_SaveVRAM("VRAM.TGA", 0, 0, VRAM_WIDTH, VRAM_HEIGHT, 1);
			break;
		case SDL_SCANCODE_F12:
			Platform_LogWarn("[CTR Native] Saving screenshot...\n");
			Platform_TakeScreenshot();
			break;
		case SDL_SCANCODE_F3:
			g_cfg_bilinearFiltering ^= 1;
			Platform_LogWarn("[CTR Native] filtering mode: %d\n", g_cfg_bilinearFiltering);
			break;
		}
	}
#endif
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

	Platform_InitScratchpad();

	int result = CTR_Main();

	Platform_Shutdown();
	return result;
}

void Platform_Init(const char *title, int width, int height)
{
	char windowName[128];

	Platform_LogInit(title);
	Platform_GetWindowName(title, windowName, sizeof(windowName));

	Platform_Log("[CTR Native] Initialising platform\n");

	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		Platform_LogError("[CTR Native] Failed to initialise SDL\n");
		Platform_LogShutdown();
		return;
	}

	s_platformInitialized = 1;

	if (!NativeRenderer_InitialiseRender(windowName, width, height, 0))
	{
		Platform_LogError("[CTR Native] Failed to initialise window\n");
		Platform_Shutdown();
		return;
	}

	if (!NativeRenderer_InitialisePSX())
	{
		Platform_LogError("[CTR Native] Failed to initialise PSX renderer state\n");
		Platform_Shutdown();
		return;
	}

	atexit(Platform_Shutdown);
	SDL_HideCursor();
	Platform_InputInit();
}

void Platform_Shutdown(void)
{
	if (s_platformInitialized == 0)
		return;

	s_platformInitialized = 0;
	Platform_InputShutdown();

	if (g_window != NULL)
	{
		SDL_DestroyWindow(g_window);
		g_window = NULL;
	}

	NativeAudio_Shutdown();
	NativeRenderer_Shutdown();

	SDL_Quit();

	Platform_LogShutdown();
}

void Platform_BeginFrame(void)
{
	// NOTE(aalhendi): Normal rendering begins from DrawOTag after the current
	// draw env is installed. Starting a host scene here clears the previous env
	// and can force the host GL driver to block before the retail render-submit path.
}

int Platform_BeginScene(void)
{
	if (s_platformBeginScene)
		return 0;

	// NOTE(aalhendi): CTR already throttles through the retail VSync/draw-sync
	// path. Do not add a second SDL swap wait; some GL drivers charge that wait
	// to the next frame's first clear instead of SDL_GL_SwapWindow.
	NativeRenderer_UpdateSwapIntervalState(0);

	NativeRenderer_BeginScene();

	if (activeDrawEnv.isbg)
	{
		const RECT16 clipenv = activeDrawEnv.clip;
		const u8 r = activeDrawEnv.r0;
		const u8 g = activeDrawEnv.g0;
		const u8 b = activeDrawEnv.b0;

		NativeRenderer_Clear(clipenv.x, clipenv.y, clipenv.w, clipenv.h, r, g, b);
	}

	s_platformBeginScene = 1;

	Platform_LogFlush();

	return 1;
}

void Platform_EndScene(void)
{
	if (!s_platformBeginScene)
		return;

	s_platformBeginScene = 0;

	NativeRenderer_EndScene();

	NativeRenderer_StoreFrameBuffer(activeDispEnv.disp.x, activeDispEnv.disp.y, activeDispEnv.disp.w, activeDispEnv.disp.h);

	NativeRenderer_SwapWindow();
}

// NOTE(aalhendi): Frame timing is handled by VSync() in the platform layer,
// matching PS1 hardware behavior. Platform_EndFrame only does buffer swap + FPS.
void Platform_EndFrame(void)
{
	Platform_EndScene();
	CalcFPS();
}

void Platform_PresentVRAMDisplay(void)
{
	Platform_BeginScene();
	NativeRenderer_PresentVRAMDisplay();
	Platform_EndFrame();
}

void Platform_PollHostEvents(void)
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_EVENT_GAMEPAD_ADDED:
			Platform_InputControllerAdded(event.gdevice.which);
			break;
		case SDL_EVENT_GAMEPAD_REMOVED:
			Platform_InputControllerRemoved(event.gdevice.which);
			break;
		case SDL_EVENT_QUIT:
			exit(0);
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			Platform_HandleWindowResize(event.window.data1, event.window.data2);
			break;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			exit(0);
			break;
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
		{
			int key = event.key.scancode;
			char down = (event.type == SDL_EVENT_KEY_UP) ? 0 : 1;

			if (key == SDL_SCANCODE_RALT)
			{
				s_hostAltKeyState = down;
			}
			else if (key == SDL_SCANCODE_RETURN)
			{
				if ((s_hostAltKeyState != 0) && (down != 0))
					Platform_HandleFullscreenToggle();
				break;
			}

			if (key == SDL_SCANCODE_RSHIFT)
				key = SDL_SCANCODE_LSHIFT;
			else if (key == SDL_SCANCODE_RCTRL)
				key = SDL_SCANCODE_LCTRL;
			else if (key == SDL_SCANCODE_RALT)
				key = SDL_SCANCODE_LALT;

			if ((key == SDL_SCANCODE_F4) && (down == 0))
			{
#ifdef CTR_INTERNAL
				Platform_LogWarn("[CTR Native] Active keyboard controller: %d\n", Platform_InputCycleKeyboardController());
#endif
				break;
			}

			Platform_HandleKey(key, down);
			break;
		}
		}
	}
}

int Platform_PollInput(void)
{
	Platform_PollHostEvents();
	Platform_InputUpdate();
	return 1;
}

int NikoGetEnterKey(void)
{
	const bool *kb = SDL_GetKeyboardState(NULL);
	return (kb && kb[SDL_SCANCODE_RETURN]) ? 1 : 0;
}

// NOTE(aalhendi): Native owns the CTR VBlank clock instead of PsyCross's
// autonomous interrupt thread. The retail-shaped VSyncCallback storage lives in
// native_libetc.c; native VSync emits that callback at each emulated VBlank.
static Uint64 s_nextVBlankCounter = 0;
static Uint64 s_vblankRemainder = 0;
static int s_nativeVBlankCount = 0;

static void Native_WaitNextVBlank(void)
{
	const Uint64 freq = SDL_GetPerformanceFrequency();
	const Uint64 hz = 60; // NOTE(aalhendi): ctr-native targets NTSC-U 926.
	const Uint64 now = SDL_GetPerformanceCounter();

	if (s_nextVBlankCounter == 0)
	{
		s_nextVBlankCounter = now;
	}
	else if ((Sint64)(now - s_nextVBlankCounter) > 0)
	{
		// NOTE(aalhendi): Retail VSync(0) waits for the next hardware VBlank;
		// it does not catch up missed VBlanks with immediate returns after a
		// host stall. Rebase stale targets so audio/video advance once per wait.
		s_nextVBlankCounter = now;
		s_vblankRemainder = 0;
	}

	s_nextVBlankCounter += freq / hz;
	s_vblankRemainder += freq % hz;
	if (s_vblankRemainder >= hz)
	{
		s_nextVBlankCounter++;
		s_vblankRemainder -= hz;
	}

	while (1)
	{
		const Uint64 now = SDL_GetPerformanceCounter();

		if ((Sint64)(s_nextVBlankCounter - now) <= 0)
			return;

		const Uint64 remaining = s_nextVBlankCounter - now;
		const Uint64 remainingMs = (remaining * 1000) / freq;

		if (remainingMs > 1)
			SDL_Delay((Uint32)(remainingMs - 1));
		else
			SDL_Delay(0);
	}
}

int VSync(int mode)
{
	int vblankCount;

	if (mode < 0)
		return s_nativeVBlankCount;

	vblankCount = (mode == 0) ? 1 : mode;

	for (int i = 0; i < vblankCount; i++)
	{
		Native_WaitNextVBlank();
		if (vsync_callback != NULL)
			vsync_callback();

		NativeAudio_StepVBlank();
		s_nativeVBlankCount++;
	}

	return s_nativeVBlankCount;
}
