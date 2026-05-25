#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ae6b0-0x800ae74c.
void MM_Characters_HideDrivers(void)
{
	char i;
	struct GameTracker *gGT = sdata->gGT;

	for (i = 0; i < 4; i++)
	{
		PushBuffer_Init(&gGT->pushBuffer[i], 0, 1);

		// player structure's instane flags, make invisible
		gGT->drivers[i]->instSelf->flags |= 0x80;
	}

	return;
}
