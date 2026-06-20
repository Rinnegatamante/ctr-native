#include <common.h>


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800afdbc-0x800b0278.
void RB_MaskWeapon_ThTick(struct Thread *maskTh)
{
	char i;
	char numPlyr;
	s16 sVar1;
	struct GameTracker *gGT;
	struct PushBuffer *pb;
	int rot;
	struct MaskHeadWeapon *mask;
	struct Instance *maskInst;
	struct Instance *maskBeamInst;
	struct Instance *driverInst;
	struct Driver *d;

	struct Instance *instCurr;

	gGT = sdata->gGT;
	numPlyr = gGT->numPlyrCurrGame;

	mask = maskTh->object;
	maskInst = maskTh->inst;
	maskBeamInst = mask->maskBeamInst;

	d = maskTh->parentThread->object;
	driverInst = maskTh->parentThread->inst;

	struct InstDrawPerPlayer *maskIdpp = INST_GETIDPP(maskInst);
	struct InstDrawPerPlayer *beamIdpp = INST_GETIDPP(maskBeamInst);

	if (d->invisibleTimer == 0)
	{
		for (i = 0; i < numPlyr; i++)
		{
			pb = &gGT->pushBuffer[i];
			maskIdpp[i].pushBuffer = pb;
			beamIdpp[i].pushBuffer = pb;
		}
	}

	else
	{
		for (i = 0; i < numPlyr; i++)
		{
			if (i == d->driverID)
				continue;

			maskIdpp[i].pushBuffer = NULL;
			beamIdpp[i].pushBuffer = NULL;
		}
	}

	// if driverInst is not reflective
	if ((driverInst->flags & 0x4000) == 0)
	{
		// mask is not reflective
		maskInst->flags &= 0xffffbfff;
	}
	// if driverInst is reflective
	else
	{
		// mask is reflective
		maskInst->flags |= 0x4000;

		// copy split line
		maskInst->vertSplit = driverInst->vertSplit;

		// mask beam is reflective
		maskBeamInst->flags |= 0x4000;

		// copy split line
		maskBeamInst->vertSplit = driverInst->vertSplit;
	}

	maskInst->depthBiasNormal = driverInst->depthBiasNormal;
	maskInst->depthBiasSecondary = driverInst->depthBiasSecondary;

	struct MaskHeadScratch *mhs = CTR_SCRATCHPAD_PTR(struct MaskHeadScratch, 0x108);

	// Set up the First pass (MaskInst)

	rot = mask->rot.y;

	mhs->posOffset[0] = (((MATH_Sin(rot) << 6) >> 0xc) * mask->scale) >> 0xc;
	mhs->posOffset[2] = (((MATH_Cos(rot) << 6) >> 0xc) * mask->scale) >> 0xc;

	mhs->posOffset[1] = R231.maskPosArr[(int)maskBeamInst->animFrame >> 0] + 0x40;

	mhs->rot[0] = 0;
	mhs->rot[1] = rot;
	mhs->rot[2] = 0;

	instCurr = maskInst;

	// First time is MaskInst,
	// Second time is BeamInst
	for (int i = 0; i < 2; i++)
	{
		if ((mask->rot.z & 1) == 0)
		{
			LHMatrix_Parent(instCurr, driverInst, (SVECTOR *)&mhs->posOffset[0]);
			ConvertRotToMatrix(&mhs->m, &mhs->rot[0]);
			MatrixRotate(&instCurr->matrix, &instCurr->matrix, &mhs->m);
		}
		else
		{
			instCurr->matrix.t[0] = (int)mask->pos.x + mhs->posOffset[0];
			instCurr->matrix.t[1] = (int)mask->pos.y + mhs->posOffset[1];
			instCurr->matrix.t[2] = (int)mask->pos.z + mhs->posOffset[2];
			ConvertRotToMatrix(&instCurr->matrix, &mhs->rot[0]);
		}

		// Set up the Second pass (BeamInst)

		mhs->posOffset[0] = 0;
		mhs->posOffset[1] = 0x40;
		mhs->posOffset[2] = 0;

		instCurr = maskBeamInst;
	}

	// === Animation ===

	// get animFrame
	sVar1 = INSTANCE_GetNumAnimFrames(maskBeamInst, 0);

	// if animation is not finished
	if ((int)maskBeamInst->animFrame < sVar1 - 1)
	{
		// increment animation frame
		maskBeamInst->animFrame += 1;
	}
	// if animation is finished
	else
	{
		// restart animation
		maskBeamInst->animFrame = 0;
	}

	// adjust rotation
	mask->rot.y += -0x100;

	// If duration is over
	if (mask->duration == 0)
	{
		ThTick_SetAndExec(maskTh, RB_MaskWeapon_FadeAway);
	}
	else
	{
		// if duration is not over
		mask->duration -= gGT->elapsedTimeMS;

		if (mask->duration < 0)
			mask->duration = 0;
	}

	// first pass
	instCurr = maskBeamInst;
	instCurr->alphaScale = 0;

	for (int i = 0; i < 2; i++)
	{
		// make visible
		instCurr->flags &= 0xffffff7f;

		instCurr->scale.x = mask->scale;
		instCurr->scale.y = mask->scale;
		instCurr->scale.z = mask->scale;

		// second pass
		instCurr = maskInst;
	}
}
