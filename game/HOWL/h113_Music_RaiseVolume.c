#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e46c-0x8002e4c0
// after "FINAL LAP!" is done
void Music_RaiseVolume(void)
{
	u32 setVolume;

	if (sdata->cseqBoolPlay != 0)
	{
		// 100% volume
		setVolume = 255;

		if ((u32)(sdata->cseqHighestIndex - 1) >= 2)
		{
			// 75% volume
			setVolume = 190;
		}

		CseqMusic_ChangeVolume(sdata->cseqHighestIndex & 0xffff, setVolume, 8);
	}
}
