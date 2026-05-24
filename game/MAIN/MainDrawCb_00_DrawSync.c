#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80034a80-0x80034aa4.
void DECOMP_MainDrawCb_DrawSync()
{
	struct GameTracker *gGT;
	gGT = sdata->gGT;

	if (gGT->bool_DrawOTag_InProgress == 1)
	{
		gGT->bool_DrawOTag_InProgress = 0;
	}

	return;
}
