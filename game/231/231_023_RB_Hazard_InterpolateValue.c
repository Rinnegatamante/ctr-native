#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ada90-0x800adb50.
int RB_Hazard_InterpolateValue(s16 currRot, s16 desiredRot, s16 rotSpeed)
{
	if (currRot == desiredRot)
		return currRot;

	int delta = (rotSpeed * sdata->gGT->elapsedTimeMS) >> 5;

	// adjust for range of "s16" [0-0xffff]
	// compared to range of degrees [0-0xfff]
	int diff = ((desiredRot - currRot) & 0xfff);
	if (diff > 0x7ff)
		diff -= 0x1000;

	// skip to end if close enough
	int diffAbs = diff;
	if (diffAbs < 0)
		diffAbs = -diffAbs;
	if (diffAbs < rotSpeed)
		return desiredRot & 0xfff;

	// interpolate
	if (diff < 0)
		currRot -= delta;
	else
		currRot += delta;
	return currRot & 0xfff;
}
