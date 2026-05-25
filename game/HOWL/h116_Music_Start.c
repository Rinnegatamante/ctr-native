#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e524-0x8002e53c
void Music_Start(u32 songID)
{
	sdata->cseqBoolPlay = true;

	// set highest song index
	sdata->cseqHighestIndex = songID & 0xffff;
}
