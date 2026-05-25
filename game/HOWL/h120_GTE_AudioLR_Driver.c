#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e5cc-0x8002e658
void GTE_AudioLR_Driver(MATRIX *matrix, struct Driver *driver, s32 *out)
{
	SVECTOR input;

	input.vx = (s16)((u32)driver->posCurr.x >> 8) - (s16)matrix->t[0];
	input.vy = (s16)((u32)driver->posCurr.y >> 8) - (s16)matrix->t[1];
	input.vz = (s16)((u32)driver->posCurr.z >> 8) - (s16)matrix->t[2];
	input.pad = 0;

	SetRotMatrix(matrix);
	gte_ldv0(&input);
	gte_rtv0();
	gte_stlvnl((VECTOR *)out);
}
