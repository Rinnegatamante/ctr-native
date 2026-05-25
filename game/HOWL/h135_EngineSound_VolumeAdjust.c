#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002fc28-0x8002fc64
int EngineSound_VolumeAdjust(int desired, int current, int step)
{
	int delta = desired - current;
	int absDelta = delta;

	if (absDelta < 0)
		absDelta = -absDelta;

	if (step < absDelta)
	{
		if (delta < 1)
			return current - step;

		return current + step;
	}

	return desired;
}
