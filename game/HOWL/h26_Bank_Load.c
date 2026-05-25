#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800297a0-0x80029824
int Bank_Load(int bankID, struct Bank *ptrBank)
{
	int numBanks = sdata->numAudioBanks;

	// if out of banks, quit
	if (numBanks >= 8)
		return 0;

	sdata->bank[numBanks].bankID = bankID & 0xffff;

	// if bank is in use, quit
	if ((sdata->bank[numBanks].flags & 3) != 0)
		return 0;

	if (Bank_Alloc(bankID, &sdata->bank[numBanks]) == 0)
		return 0;

	// starting to think this isn't really a bank...
	ptrBank->bankID = sdata->numAudioBanks++;
	return 1;
}
