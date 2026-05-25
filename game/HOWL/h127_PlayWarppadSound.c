#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e994-0x8002e9c0
void PlayWarppadSound(u32 distance)
{
	CalculateVolumeFromDistance((u32 *)&sdata->SoundFadeInput[0].soundID_soundCount, 0x98, distance);
}
