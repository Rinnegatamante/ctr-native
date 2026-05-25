#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e658-0x8002e690
int GTE_GetSquaredLength(s32 *vec)
{
	MTC2(vec[0], 9);
	MTC2(vec[1], 10);
	MTC2(vec[2], 11);
	gte_sqr0();

	return MFC2(25) + MFC2(26) + MFC2(27);
}
