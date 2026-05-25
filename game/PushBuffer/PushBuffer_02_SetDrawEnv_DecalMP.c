#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80042974-0x80042a8c.
void PushBuffer_SetDrawEnv_DecalMP(void *ot, struct DB *backBuffer, RECT *viewport, s16 offsetX, s16 offsetY, u8 dtd, u8 dfe, u8 isbg, u8 tpageUpper,
                                   u8 tpageLower)
{
	void *p;
	DRAWENV newDrawEnv;

	// Copy DrawEnv from gGT->backBuffer
	int *dst = (int *)&newDrawEnv;
	int *src = (int *)&backBuffer->drawEnv;

	for (u32 i = 0; i < sizeof(DRAWENV) / 4; i++)
		dst[i] = src[i];

	// Now modify DrawEnv...

	// RECT viewport (startX, startY, endX, endY)
	newDrawEnv.clip.x = viewport->x;
	newDrawEnv.clip.y = viewport->y;
	newDrawEnv.clip.w = viewport->w;
	newDrawEnv.clip.h = viewport->h;

	newDrawEnv.ofs[1] = offsetY;

	// tpage
	newDrawEnv.tpage = (u16)((tpageUpper << 8) | tpageLower);

	// dtd (dithering)
	newDrawEnv.dtd = dtd;

	// dfe (blocked or permitted)
	newDrawEnv.dfe = dfe;

	// isbg (always 0)
	newDrawEnv.isbg = isbg;

	p = backBuffer->primMem.curr;
	void *prim = NULL;

	// curr < endMin100
	if (p <= backBuffer->primMem.endMin100)
	{
		// advance curr
		backBuffer->primMem.curr = (void *)((u32)backBuffer->primMem.curr + 0x40);

		prim = p;
	}

	if (prim == NULL)
		return;

	// ofs[X]
	newDrawEnv.ofs[0] = offsetX;

	// DrawEnv just built
	SetDrawEnv(prim, &newDrawEnv);

	// This doesn't really draw a primitive,
	// it links the ptrOT from the camera,
	// into the ptrOT of backBuffer DB, allowing
	// this camera's primitives to draw
	AddPrim(ot, prim);
}
