#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002b4d0-0x8002b508
void Smart_EnterCriticalSection(void)
{
	int count = sdata->criticalSectionCount;

	sdata->criticalSectionCount = count + 1;

	if (count == 0)
		EnterCriticalSection();
}
