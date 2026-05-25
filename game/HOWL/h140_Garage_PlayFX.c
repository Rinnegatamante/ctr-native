#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80030404-0x800304b8.
void Garage_PlayFX(u32 soundId, char charId)
{
	if (charId < PINSTRIPE)
	{
		// if sound == BIRD_RANDOM
		if (soundId == 0xf6)
		{
			sdata->audioRNG = ((sdata->audioRNG >> 3) + sdata->audioRNG * 0x20000000) * 5 + 1;
			soundId = (sdata->audioRNG % 3) + 0xf3;
		}

		DECOMP_OtherFX_Play_LowLevel(soundId & 0xffff, 1, sdata->garageSoundPool[charId].volume << 0x10 | sdata->garageSoundPool[charId].LR | 0x8000);
	}
}
