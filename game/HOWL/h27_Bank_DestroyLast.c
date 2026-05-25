#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80029824-0x80029870
int Bank_DestroyLast()
{
	if (sdata->numAudioBanks == 0)
		return 0;

	Bank_Destroy(&sdata->bank[--sdata->numAudioBanks]);
	return 1;
}
