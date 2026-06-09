/*
 * Derived from REDRIVER2/PsyCross MIT source:
 * externals/PsyCross/src/psx/LIBAPI.C
 * See THIRD_PARTY_NOTICES.md for copyright and license details.
 */

#include <macros.h>

#include <SDL3/SDL.h>
#include <psx/libapi.h>

// CTR's retail timer reads root counter 1 once per VSync and converts units
// through divisor 0x147e before MainFrame_GameLogic scales to elapsedTimeMS.
// Native emits VSync at 60 Hz, so feed that retail path 263 scanline ticks per
// VBlank; two VBlanks then land on the normal 0x20 gameplay frame step.
#define CTR_NATIVE_RCNT1_HZ (263u * 60u)

global_variable u64 s_rootCounterBase = 0;

int SetRCnt(int spec, unsigned short target, int mode)
{
	spec &= 0xffff;
	if (spec > 2)
		return 0;

	(void)target;
	(void)mode;
	return 1;
}

int GetRCnt(int spec)
{
	const u64 freq = SDL_GetPerformanceFrequency();
	const u64 now = SDL_GetPerformanceCounter();
	u64 elapsed;
	u64 counts;

	(void)spec;

	if (s_rootCounterBase == 0)
		s_rootCounterBase = now;

	elapsed = now - s_rootCounterBase;
	counts = ((elapsed / freq) * CTR_NATIVE_RCNT1_HZ) + (((elapsed % freq) * CTR_NATIVE_RCNT1_HZ) / freq);
	if (counts > 0x7fffffff)
		return 0x7fffffff;

	return (int)counts;
}

int StartRCnt(int spec)
{
	spec &= 0xffff;
	if (spec > 2)
		return 0;

	return 1;
}

int StopRCnt(int spec)
{
	(void)spec;
	return 0;
}

int ResetRCnt(int spec)
{
	(void)spec;

	s_rootCounterBase = SDL_GetPerformanceCounter();
	return 0;
}

int OpenEvent(unsigned int event, int spec, int mode, long (*func)())
{
	(void)event;
	(void)spec;
	(void)mode;
	(void)func;
	return 0;
}

int CloseEvent(unsigned int event)
{
	(void)event;
	return 0;
}

int EnableEvent(unsigned int event)
{
	(void)event;
	return 0;
}

int TestEvent(unsigned int event)
{
	(void)event;
	return 0;
}

void InitCARD(int val)
{
	(void)val;
}

int StartCARD(void)
{
	return 0;
}

int StopCARD(void)
{
	return 0;
}

void _bu_init(void)
{
}
