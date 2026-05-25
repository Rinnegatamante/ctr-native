#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002a9d8-0x8002a9f0
void SongPool_Volume(struct Song *song, int newVol, int newStep, int boolImm)
{
	// if immediate change request,
	// without fading volume
	if (boolImm != 0)
	{
		song->vol_Curr = newVol;
	}

	song->vol_New = newVol;
	song->vol_StepRate = newStep;
}
