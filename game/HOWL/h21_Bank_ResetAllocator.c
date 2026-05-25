#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800292e0-0x800292fc
void Bank_ResetAllocator()
{
	sdata->numAudioBanks = 0;
	sdata->audioAllocPtr = 0x202;
	sdata->bankLoadStage = 4; // Stage 4: Finished
}
