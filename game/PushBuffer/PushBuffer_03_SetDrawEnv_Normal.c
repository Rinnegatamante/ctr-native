#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80042a8c-0x80042c04.
void PushBuffer_SetDrawEnv_Normal(void *ot, struct PushBuffer *pb, struct DB *backBuffer, s16 *copyDrawEnvNULL, int isbg)
{
	DRAWENV newDrawEnv;

	int *dst = (int *)&newDrawEnv;
	int *src = (int *)&backBuffer->drawEnv;

	for (u32 i = 0; i < sizeof(DRAWENV) / 4; i++)
		dst[i] = src[i];

	// always?
	if (copyDrawEnvNULL == 0)
	{
		newDrawEnv.clip.x += pb->rect.x;
		newDrawEnv.clip.y += pb->rect.y;
		newDrawEnv.clip.w = pb->rect.w;
		newDrawEnv.clip.h = pb->rect.h;
		newDrawEnv.ofs[0] += pb->rect.x;
		newDrawEnv.ofs[1] += pb->rect.y;
	}

	else
	{
		newDrawEnv.clip.x = copyDrawEnvNULL[0];
		newDrawEnv.clip.y = copyDrawEnvNULL[1];
		newDrawEnv.clip.w = copyDrawEnvNULL[2];
		newDrawEnv.clip.h = copyDrawEnvNULL[3];
		newDrawEnv.ofs[0] = copyDrawEnvNULL[0];
		newDrawEnv.ofs[1] = copyDrawEnvNULL[1];
	}

	newDrawEnv.isbg = isbg;

	void *p = backBuffer->primMem.curr;
	if (p <= backBuffer->primMem.endMin100)
	{
		backBuffer->primMem.curr = (void *)((u32)backBuffer->primMem.curr + 0x40);

		SetDrawEnv(p, &newDrawEnv);

		// This doesn't really draw a primitive,
		// it links the ptrOT from the camera,
		// into the ptrOT of backBuffer DB, allowing
		// this camera's primitives to draw
		AddPrim(ot, p);
	}
}
