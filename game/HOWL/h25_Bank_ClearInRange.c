#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80029730-0x800297a0
void Bank_ClearInRange(u16 min, u16 max)
{
	int i;
	u16 end = min + max;
	struct SpuAddrEntry *sae;
	sae = &sdata->howl_spuAddrs[0];

	for (i = 0; i < sdata->ptrHowlHeader->numSpuAddrs; i++)
	{
		if (sae[i].spuAddr < min)
			continue;
		if (sae[i].spuAddr >= end)
			continue;
		sae[i].spuAddr = 0;
	}
}
