#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800691e4-0x8006924c.
void VehTalkMask_PlayXA(struct Instance *i, int id)
{
	struct Driver *d = sdata->gGT->drivers[0];

	if (d != 0)
	{
		int boolGoodGuy = VehPickupItem_MaskBoolGoodGuy(d);

		if (boolGoodGuy == 0)
			id += 0x1f;
	}

	CDSYS_XAPlay(CDSYS_XA_TYPE_EXTRA, id);
}
