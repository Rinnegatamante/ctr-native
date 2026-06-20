#include <common.h>

_Static_assert(sizeof(struct WarpPad) == 0x78);
_Static_assert(offsetof(struct WarpPad, lightDirGem) == 0x50);
_Static_assert(offsetof(struct WarpPad, digit10s) == 0x68);
_Static_assert(offsetof(struct WarpPad, levelID) == 0x6c);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abbdc-0x800abd80.
void AH_WarpPad_AllWarppadNum()
{
	struct WarpPad *wp;
	struct ModelHeader *mh;
	struct Instance *inst;

	struct Thread *t = sdata->gGT->threadBuckets[WARPPAD].thread;

	for (t; t != 0; t = t->siblingThread)
	{
		wp = t->object;

		// DCxDemo says:
		// 1 to 8 is taken from mpk i guess, 0, 9 and X are seprate models

		if ((wp->inst[2] != 0) && (wp->digit1s != 0) && (wp->digit1s != 9))
		{
			inst = wp->inst[2];
			mh = &inst->model->headers[0];
			AH_WarpPad_SetNumModelData(inst, &mh[wp->digit1s - 1]);
		}

		if ((wp->inst[3] != 0) && (wp->digit10s != 0))
		{
			inst = wp->inst[3];
			mh = &inst->model->headers[0];
			AH_WarpPad_SetNumModelData(inst, mh);
		}
	}
}

void AH_WarpPad_SetNumModelData(struct Instance *inst, struct ModelHeader *mh)
{
	struct InstDrawPerPlayer *idpp = INST_GETIDPP(inst);

	idpp[0].ptrCommandList = mh->ptrCommandList;
	idpp[0].ptrColorLayout = (u32)mh->ptrColors;
	idpp[0].ptrTexLayout = mh->ptrTexLayout;
	idpp[0].ptrCurrFrame = mh->ptrFrameData;
}
