#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80047224-0x80047230.
void RefreshCard_GetNumGhostsTotal(void)
{
	*(s16 *)&sdata->numGhostProfilesSaved = 0;
}
