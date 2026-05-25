#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800304b8-0x80030694.
void Garage_LerpFX(void)
{
	struct GarageFX *garageSounds = sdata->garageSoundPool;
	u32 *audioPtrRef;

	for (int i = 0; i < 8; ++i, ++garageSounds)
	{
		s16 targetVolume, targetLR;
		char cVar1 = garageSounds->gsp_curr;

		if (cVar1 == GSP_CENTER)
		{
			targetVolume = 0xff;
			targetLR = 0x80;
		}
		else if (cVar1 == GSP_LEFT)
		{
			targetVolume = 100;
			targetLR = 0x3c;
		}
		else if (cVar1 == GSP_RIGHT)
		{
			targetVolume = 100;
			targetLR = 0xc3;
		}
		else
		{
			targetVolume = 0;
			targetLR = garageSounds->LR;
		}

		if (targetLR == garageSounds->LR && targetVolume == garageSounds->volume)
			continue;

		if (targetVolume != garageSounds->volume)
		{
			s16 delta = (garageSounds->volume < targetVolume) ? 8 : -8;
			garageSounds->volume += delta;
			if ((delta > 0 && garageSounds->volume > targetVolume) || (delta < 0 && garageSounds->volume < targetVolume))
			{
				garageSounds->volume = targetVolume;
			}
		}

		if (targetLR != garageSounds->LR)
		{
			s16 delta = (garageSounds->LR < targetLR) ? 2 : -2;
			garageSounds->LR += delta;
			if ((delta > 0 && garageSounds->LR > targetLR) || (delta < 0 && garageSounds->LR < targetLR))
			{
				garageSounds->LR = targetLR;
			}
		}

		audioPtrRef = (u32 *)&garageSounds->audioPtr;
		if (sdata->garageSoundIDs[i] != 0)
		{
			DECOMP_OtherFX_RecycleNew(audioPtrRef, sdata->garageSoundIDs[i], ((int)garageSounds->volume << 0x10) | (int)garageSounds->LR | 0x8000U);
		}

		if (targetLR == garageSounds->LR && targetVolume == garageSounds->volume)
		{
			garageSounds->gsp_prev = garageSounds->gsp_curr;

			if (garageSounds->gsp_curr == GSP_GONE)
				DECOMP_OtherFX_RecycleMute((int *)audioPtrRef);
		}
	}
}
