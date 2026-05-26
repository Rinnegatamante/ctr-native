#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800471c4-0x800471e8.
void RefreshCard_SetScreenText(int screenText)
{
	sdata->mcScreenText = screenText;
	RefreshCard_Unknown1();
}
