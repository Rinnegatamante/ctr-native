/*
 * Derived from REDRIVER2/PsyCross MIT source:
 * externals/PsyCross/src/psx/LIBETC.C
 * See THIRD_PARTY_NOTICES.md for copyright and license details.
 */

#include <psx/libetc.h>
#include <platform/native_psyx_shell.h>

void (*vsync_callback)(void) = NULL;

int VSyncCallback(void (*func)(void))
{
	int old = (int)vsync_callback;

	vsync_callback = func;
	return old;
}

int StopCallback(void)
{
	return 0;
}

int ResetCallback(void)
{
	int old = (int)vsync_callback;

	vsync_callback = NULL;
	return old;
}

int SetVideoMode(int mode)
{
	return PsyX_Sys_SetVMode(mode);
}

int GetVideoMode(void)
{
	return g_vmode;
}
