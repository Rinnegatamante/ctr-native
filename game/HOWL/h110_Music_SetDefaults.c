#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e338-0x8002e350
void Music_SetDefaults(void)
{
	// no music playing
	sdata->cseqBoolPlay = false;
	sdata->cseqHighestIndex = -1;
	sdata->cseqTempo = 0;
}
