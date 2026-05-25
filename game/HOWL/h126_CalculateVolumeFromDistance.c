#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e84c-0x8002e994
void CalculateVolumeFromDistance(u32 *soundIDCount, u32 soundID, int distance)
{
	u32 volume;

	if (distance < 6000)
	{
		if ((*soundIDCount != 0) && ((*soundIDCount & 0xffff) != soundID))
		{
			DECOMP_OtherFX_Stop1(*soundIDCount);
			*soundIDCount = 0;
		}

		if (distance < 301)
			volume = 0xff;
		else
			volume = VehCalc_MapToRange(distance, 300, 6000, 0xff, 0);

		if (soundID != (u32)-1)
		{
			if (*soundIDCount == 0)
			{
				*soundIDCount = DECOMP_OtherFX_Play_LowLevel(soundID & 0xffff, 0, ((volume & 0xff) << 0x10) | 0x8080);
			}
			else
			{
				if (soundID == 0x89)
				{
					int distort = ((u32)sdata->gGT->frameTimer_VsyncCallback >> 2 & 0x7f) - 0x40;
					if (distort < 0)
						distort = -distort;

					volume = ((volume & 0xff) << 0x10) | (((distort + 100U) & 0xff) << 8) | 0x80;
				}
				else
				{
					volume = ((volume & 0xff) << 0x10) | 0x8080;
				}

				DECOMP_OtherFX_Modify(*soundIDCount, volume);
			}
		}
	}
	else if (*soundIDCount != 0)
	{
		DECOMP_OtherFX_Stop1(*soundIDCount);
		*soundIDCount = 0;
	}
}
