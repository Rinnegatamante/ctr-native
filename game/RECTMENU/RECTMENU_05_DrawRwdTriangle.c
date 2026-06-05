#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800453e8-0x80045534.
void RECTMENU_DrawRwdTriangle(s16 *position, char *color, u_long *otMem, struct PrimMem *primMem)
{
	POLY_G4 *p;
	void *primmemCurr;

	primmemCurr = primMem->curr;
	p = 0;

	if (primmemCurr <= primMem->endMin100)
	{
		p = primmemCurr;
		primMem->curr = p + 1;
	}

	if (p != 0)
	{
		// RGB
		p->r0 = (u8)color[0x0];
		p->g0 = (u8)color[0x1];
		p->b0 = (u8)color[0x2];

		p->r1 = (u8)color[0x4];
		p->g1 = (u8)color[0x5];
		p->b1 = (u8)color[0x6];

		p->r2 = (u8)color[0x0];
		p->g2 = (u8)color[0x1];
		p->b2 = (u8)color[0x2];

		p->r3 = (u8)color[0x8];
		p->g3 = (u8)color[0x9];
		p->b3 = (u8)color[0xa];

		// rest of the primitive (four xy)
		p->x0 = position[0];
		p->y0 = position[1] - 1;

		p->x1 = position[2];
		p->y1 = position[3];

		p->x2 = position[0];
		p->y2 = position[1];

		p->x3 = position[4];
		p->y3 = position[5];

		setPolyG4(p);
		AddPrim(otMem, p);
	}
	return;
}
