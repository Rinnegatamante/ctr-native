#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005b0c4-0x8005b0f4.
int VehFrameInst_GetStartFrame(int animIndex, int numFrames)
{
	switch (animIndex)
	{
	// midpoint
	case 0:
		return numFrames >> 1;

	// end
	case 4:
		return (numFrames - 1);

	// start
	default:
		return 0;
	}
}
