#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003074c-0x80030778.
void Garage_Leave(void)
{
	int i;
	struct GarageFX *garageSounds = sdata->garageSoundPool;

	for (i = 0; i < 8; i++)
	{
		garageSounds[i].gsp_curr = GSP_GONE;
	}

	return;
}
