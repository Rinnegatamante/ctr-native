#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80047d64-0x80047da8.
void RefreshCard_Entry(void)
{
	if ((sdata->gGT->gameMode1 & DEBUG_MENU) == 0)
	{
		RefreshCard_Unknown4();
		RefreshCard_Unknown3();
	}
}
