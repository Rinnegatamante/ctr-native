#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e7bc-0x8002e84c
int GTE_GetSquaredDistance(s16 *pos1, s16 *pos2)
{
	int dx = pos1[0] - pos2[0];
	int dy = pos1[1] - pos2[1];
	int dz = pos1[2] - pos2[2];

	MTC2(dx, 9);
	MTC2(dy, 10);
	MTC2(dz, 11);
	gte_sqr0();

	return MFC2(25) + MFC2(26) + MFC2(27);
}
