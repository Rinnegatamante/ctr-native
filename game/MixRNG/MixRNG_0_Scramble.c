#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003ea28-0x8003ea6c
int MixRNG_Scramble()
{
	// random algorithm for seemingly-random numbers
	sdata->randomNumber = (sdata->randomNumber * 0x6255 + 0x3619) & 0xffff;
	return sdata->randomNumber;
}
