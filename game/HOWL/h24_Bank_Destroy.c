#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800296c4-0x80029730
void Bank_Destroy(struct Bank *ptrLastBank)
{
	u16 flags;

	if (sdata->boolAudioEnabled == 0)
		return;

	flags = ptrLastBank->flags;

	Bank_ClearInRange(ptrLastBank->min, ptrLastBank->max);

	if ((flags & 1) == 0)
	{
		// this works cause Bank_Destroy
		// is only called on the "last" bank
		sdata->audioAllocPtr = ptrLastBank->min;
	}

	ptrLastBank->flags = flags & ~(2);
}
