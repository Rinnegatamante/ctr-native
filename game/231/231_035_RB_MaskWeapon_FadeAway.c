#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 231 0x800afb70-0x800afdbc.
void RB_MaskWeapon_FadeAway(struct Thread *t)
{
	s16 totalTime;
	MATRIX *m;
	struct Instance *inst;
	struct Instance *driverInst;
	struct Instance *maskBeamInst;
	struct MaskHeadWeapon *mask;

	inst = t->inst;
	mask = inst->thread->object;
	driverInst = t->parentThread->inst;
	maskBeamInst = mask->maskBeamInst;

	struct MaskHeadScratch *mhs = CTR_SCRATCHPAD_PTR(struct MaskHeadScratch, 0x108);

	// Set up First pass (maskInst)

	int durationAdjusted = ((int)(mask->duration >> 5) * -4 + 0x40);
	mhs->posOffset.x = ((durationAdjusted * MATH_Sin(mask->rot.y)) >> 0xc);
	mhs->posOffset.z = ((durationAdjusted * MATH_Cos(mask->rot.y)) >> 0xc);
	mhs->posOffset.y = 0x40;

	mask->rot.y += -0x100;

	struct Instance *instCurr;
	instCurr = inst;

	// First time is MaskInst,
	// Second time is BeamInst
	for (int i = 0; i < 2; i++)
	{
		LHMatrix_Parent(instCurr, driverInst, (SVECTOR *)&mhs->posOffset);

		instCurr->scale.x += -0x100;
		instCurr->scale.y += -0x100;
		instCurr->scale.z += -0x100;

		// position offset
		mhs->posOffset.x = 0;
		mhs->posOffset.z = 0;

		instCurr = maskBeamInst;
	}

	mhs->rot.x = 0;
	mhs->rot.y = mask->rot.y;
	mhs->rot.z = 0;
	ConvertRotToMatrix(&mhs->m, &mhs->rot);

	m = &maskBeamInst->matrix;
	MatrixRotate(m, m, &mhs->m);

	if (maskBeamInst->alphaScale < 0x1000)
		maskBeamInst->alphaScale += 0x200;

	totalTime = mask->duration;

	if (totalTime < 0x200)
	{
		totalTime += sdata->gGT->elapsedTimeMS;

		if (totalTime > 0x200)
			totalTime = 0x200;

		mask->duration = totalTime;
		return;
	}

	// mask is now dead
	INSTANCE_Death(maskBeamInst);
	t->flags |= THREAD_FLAG_DEAD;
	return;
}
