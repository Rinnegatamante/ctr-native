#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e53c-0x8002e550
void Music_End(void)
{
	sdata->cseqBoolPlay = false;

	// no songs are playing
	sdata->cseqHighestIndex = -1;
}
