#include <common.h>

void MatrixRotate(MATRIX *dst, MATRIX *src, MATRIX *rot)
{
	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8006c3b0-0x8006c430
	MulMatrix0(src, rot, dst);
}
