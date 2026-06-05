#define _CRT_SECURE_NO_WARNINGS
#define CTR_NATIVE
#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

#if __GNUC__
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#define _EnterCriticalSection(x)
#define EnterCriticalSection(x)
#define ExitCriticalSection()
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
#include "platform/native_assets.h"
#include "platform/native_log.h"
#include "platform/native_replay_scheduler.h"
#include "platform/native_savestate.h"

#ifndef __GNUC__
#define _Static_assert(x)
#define __attribute__(x)
#endif

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

#include "game_includes.h"

#include "game/zGlobal_RDATA.c"
#include "game/zGlobal_DATA.c"
#include "game/zGlobal_SDATA.c"

#undef RECT

#include "platform/native_assets.c"
#include "platform/native_audio.c"
#include "platform/native_memory.c"
#include "platform/native_checkpoint.c"
#include "platform/native_checkpoint_file.c"
#include "platform/native_cd.c"
#include "platform/native_gpu.c"
#include "platform/native_gte_core.c"
#include "platform/native_glad.c"
#include "platform/native_input.c"
#include "platform/native_inline_c.c"
#include "platform/native_libapi.c"
#include "platform/native_libetc.c"
#include "platform/native_libgte.c"
#include "platform/native_libgpu.c"
#include "platform/native_libpad.c"
#include "platform/native_libspu.c"
#include "platform/native_log.c"
#include "platform/native_memcard.c"
#include "platform/native_platform.c"
#include "platform/native_replay_scheduler.c"
#include "platform/native_renderer.c"
#include "platform/native_savestate.c"
#include "platform/native_state.c"
#include "platform/native_str.c"

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

	const char *sdlBasePath = SDL_GetBasePath();
	printf("[CTR Native] SDL base path: %s\n", sdlBasePath ? sdlBasePath : "(null)");
	fflush(stdout);

	char baseDir[1024];

	if (sdlBasePath)
	{
		snprintf(baseDir, sizeof(baseDir), "%s", sdlBasePath);
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

	if (!NativeAssets_Validate())
		return 1;

#if defined(CTR_INTERNAL)
	if (NativeReplayScheduler_PrepareReportFromArgs(argc, argv) != 0)
		return 1;
#endif

#ifdef USE_16BY9
	printf("[CTR Native] Widescreen\n");
	Platform_Init("Crash Team Racing", 1280, 720);
#else
	printf("[CTR Native] 4:3\n");
	Platform_Init("Crash Team Racing", 800, 600);
#endif

	Platform_InitScratchpad();
	Platform_RepairResidentPointers(0);

#if defined(CTR_INTERNAL)
	if (NativeReplayScheduler_ConfigureFromArgs(argc, argv) != 0)
	{
		Platform_LogFlush();
		Platform_Shutdown();
		return 1;
	}
#else
	(void)argc;
	(void)argv;
#endif

	int result = CTR_Main();

	Platform_Shutdown();
	return result;
}
