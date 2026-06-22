#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003d264-0x8003d328.
MATRIX *MATH_HitboxMatrix(MATRIX *output, MATRIX *input)
{
	u32 *in = (u32 *)input;
	u32 *out = (u32 *)output;

	u32 r11r12 = (in[0] & 0xffff) | (in[1] & 0xffff0000);
	u32 r13r21 = (in[3] & 0xffff) | (in[0] & 0xffff0000);
	u32 r22r23 = (in[2] & 0xffff) | (in[3] & 0xffff0000);
	u32 r31r32 = (in[1] & 0xffff) | (in[2] & 0xffff0000);
	s16 r33 = *(s16 *)&in[4];

	CTC2(r11r12, 0);
	CTC2(r13r21, 1);
	CTC2(r22r23, 2);
	CTC2(r31r32, 3);
	CTC2((s32)r33, 4);

	MTC2(((0u - (u32)input->t[0]) & 0xffff) | ((0u - (u32)input->t[1]) << 16), 0);
	MTC2(0u - (u32)input->t[2], 1);
	gte_rtv0();

	out[0] = r11r12;
	out[1] = r13r21;
	out[2] = r22r23;
	out[3] = r31r32;
	*(s16 *)&out[4] = r33;
	CTR_GteStoreMAC(output->t);

	return output;
}
