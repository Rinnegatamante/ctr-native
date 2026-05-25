#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80029870-0x800298e4
void Bank_DestroyUntilIndex(int index)
{
	struct Bank *ptrLastBank;
	u16 bankID = index;

	while (sdata->numAudioBanks != 0)
	{
		ptrLastBank = &sdata->bank[sdata->numAudioBanks - 1];

		if ((u16)ptrLastBank->bankID == bankID)
			return;

		Bank_DestroyLast();
	}
}
