#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e418-0x8002e46c
// happens during "FINAL LAP!"
void Music_LowerVolume(void)
{
	u32 setVolume;

	if (sdata->cseqBoolPlay != 0)
	{
		// 50% volume
		setVolume = 150;

		if ((u32)(sdata->cseqHighestIndex - 1) >= 2)
		{
			// 25% volume
			setVolume = 90;
		}

		CseqMusic_ChangeVolume(sdata->cseqHighestIndex & 0xffff, setVolume, 8);
	}
}
