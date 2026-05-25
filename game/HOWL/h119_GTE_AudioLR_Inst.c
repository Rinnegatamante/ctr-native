#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e55c-0x8002e5cc
void GTE_AudioLR_Inst(MATRIX *matrix, s32 *vec)
{
	SVECTOR input;

	input.vx = (s16)vec[0];
	input.vy = (s16)vec[1];
	input.vz = (s16)vec[2];
	input.pad = 0;

	SetRotMatrix(matrix);
	gte_ldv0(&input);
	gte_rtv0();
	gte_stlvnl((VECTOR *)vec);
}
