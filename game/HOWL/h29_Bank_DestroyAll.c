#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800298e4-0x8002991c
void Bank_DestroyAll()
{
	struct Bank *ptrLastBank;

	while (sdata->numAudioBanks != 0)
	{
		Bank_DestroyLast();
	}
}
