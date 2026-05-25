#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e9c0-0x8002ea44
void Level_SoundLoopSet(int *soundIDCount, u32 soundID, u32 volume)
{
	if (volume == 0)
	{
		if (*soundIDCount != 0)
		{
			DECOMP_OtherFX_Stop1(*soundIDCount);
			*soundIDCount = 0;
		}
	}
	else if (*soundIDCount == 0)
	{
		*soundIDCount = DECOMP_OtherFX_Play_LowLevel(soundID & 0xffff, 0, ((volume & 0xff) << 0x10) | 0x8080);
	}
	else
	{
		DECOMP_OtherFX_Modify(*soundIDCount, ((volume & 0xff) << 0x10) | 0x8080);
	}
}
