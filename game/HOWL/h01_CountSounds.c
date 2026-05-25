#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002843c-0x80028468
int CountSounds(void)
{
	// watch it increase when scrolling in main menu
	sdata->countSounds += 1;
	if (sdata->countSounds == 0)
	{
		sdata->countSounds = 1;
	}
	return sdata->countSounds;
}
