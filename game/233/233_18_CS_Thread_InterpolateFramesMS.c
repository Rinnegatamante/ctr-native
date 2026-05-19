#include <common.h>

void DECOMP_CS_Thread_InterpolateFramesMS(struct Thread *t)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Instance *inst = t->inst;
	struct PrimMem *primMem;
	u_int *prim;
	u_int *end;
	short curr[4] = {0};
	short next[4] = {0};
	int depth;

	if (inst == NULL)
		return;

	DECOMP_CS_Instance_GetFrameData(inst, inst->animIndex, inst->animFrame, (u_short *)curr, NULL, 0);
	DECOMP_CS_Instance_GetFrameData(inst, inst->animIndex, inst->animFrame, (u_short *)next, NULL, 1);

	curr[0] += inst->matrix.t[0];
	curr[1] += inst->matrix.t[1];
	curr[2] += inst->matrix.t[2];

	next[0] += inst->matrix.t[0];
	next[1] += inst->matrix.t[1];
	next[2] += inst->matrix.t[2];

	primMem = &gGT->backBuffer->primMem;
	prim = (u_int *)primMem->curr;
	end = (u_int *)primMem->endMin100;

	if (prim + 6 >= end)
		return;

	gte_SetRotMatrix(&gGT->pushBuffer[0].matrix_ViewProj);
	gte_SetTransMatrix(&gGT->pushBuffer[0].matrix_ViewProj);

	MTC2((curr[0] & 0xffff) | (curr[1] << 16), 0);
	MTC2((u_short)curr[2], 1);
	MTC2((next[0] & 0xffff) | (next[1] << 16), 2);
	MTC2((u_short)next[2], 3);
	gte_rtpt();

	prim[4] = MFC2(12);
	prim[5] = MFC2(13);

	depth = MFC2(17);
	if ((u_int)(depth - 1) < 0x11ff)
	{
		u_int color = 0x3f;
		int otIndex;
		u_int *ot;

		prim[1] = 0xe1000a20;
		prim[2] = 0;

		if (depth > 0xa00)
		{
			int fade = (0x1200 - depth) * 0x3f;

			color = fade >> 11;
			if (fade < 0)
				color = (fade + 0x7ff) >> 11;
		}

		prim[3] = color | (color << 8) | (color << 16) | 0x42000000;

		otIndex = depth >> 6;
		if (otIndex > 0x3ff)
			otIndex = 0x3ff;

		ot = (u_int *)&gGT->pushBuffer[0].ptrOT[otIndex];
		prim[0] = (*ot & 0xffffff) | 0x05000000;
		*ot = (u_int)prim & 0xffffff;
		prim += 6;
	}

	primMem->curr = prim;
}
