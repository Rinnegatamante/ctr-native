#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002b508-0x8002b540
void Smart_ExitCriticalSection(void)
{
	int count = sdata->criticalSectionCount;

	if (count == 0)
		return;

	count--;
	sdata->criticalSectionCount = count;

	if (count == 0)
		ExitCriticalSection();
}
