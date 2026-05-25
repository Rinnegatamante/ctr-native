#include <common.h>

static void VehFrameProc_Driving_SpawnBurnSmoke(struct Driver *d)
{
	struct Particle *p = Particle_Init(0, sdata->gGT->iconGroup[1], &data.emSet_BurnSmoke[0]);

	if (p != NULL)
	{
		p->unk18 = d->instSelf->unk50;
		p->driverInst = d->instSelf;
		p->unk19 = d->driverID;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005b178-0x8005b510
void VehFrameProc_Driving(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;
	u8 desiredAnim = 0;
	int numFrames;

	if ((d->instTntRecv == NULL) && (d->kartState != KS_WARP_PAD))
	{
		if (d->fireSpeed < 0)
		{
			desiredAnim = d->speedApprox < 1;
		}

		if (((d->jumpHeightCurr > 0x600) || (inst->animIndex == 3)) && (d->posCurr.y - d->quadBlockHeight > 0x8000))
		{
			desiredAnim = 3;
		}
	}

	numFrames = VehFrameInst_GetNumAnimFrames(inst, inst->animIndex);
	if (numFrames <= 0)
	{
		return;
	}

	if (desiredAnim != inst->animIndex)
	{
		u8 currAnim = inst->animIndex;
		int startFrame;

		if (currAnim == 2)
		{
			startFrame = VehFrameInst_GetNumAnimFrames(inst, 2) - 1;
		}
		else
		{
			startFrame = VehFrameInst_GetStartFrame(currAnim, numFrames);
		}

		if (inst->animFrame == startFrame)
		{
			numFrames = VehFrameInst_GetNumAnimFrames(inst, desiredAnim);
			if (numFrames <= 0)
			{
				return;
			}

			inst->animIndex = desiredAnim;
			inst->animFrame = VehFrameInst_GetStartFrame(desiredAnim, numFrames);
			d->matrixArray = 0;
			d->matrixIndex = 0;
		}
		else
		{
			int speed = 2;

			if (currAnim == 0)
			{
				speed = 6;
			}
			else if (currAnim == 2)
			{
				speed = 1;
				d->matrixIndex = inst->animFrame;
			}

			inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, speed, startFrame);

			if ((u32)(inst->animIndex - 2) < 2)
			{
				d->matrixIndex = (u8)inst->animFrame;
				if (d->matrixIndex == 0)
				{
					d->matrixArray = 0;
					d->matrixIndex = 0;
				}
			}

			return;
		}
	}

	if (desiredAnim == 0)
	{
		int targetFrame = numFrames >> 1;

		if (d->instTntRecv == NULL)
		{
			s16 burnTimer = d->burnTimer;

			if ((burnTimer != 0) && (burnTimer < 0x1e0))
			{
				targetFrame += (((burnTimer >> 5) % 5) << 2) - 8;
				inst->animFrame = targetFrame;
				VehFrameProc_Driving_SpawnBurnSmoke(d);
			}
			else
			{
				int turnMin = -0x40;
				int turnMax = 0x40;
				int turnState = d->ampTurnState;

				if ((d->actionsFlagSet & 8) == 0)
				{
					turnMax = (u8)d->const_TurnRate;
					turnMin = -turnMax;
					turnState = d->simpTurnState;
				}

				targetFrame = VehCalc_MapToRange(-turnState, turnMin, turnMax, 0, numFrames - 1);
			}
		}

		inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, 1, targetFrame);
		return;
	}

	if (desiredAnim == 3)
	{
		s16 characterID;
		u8 matrixArray;

		inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, 1, numFrames - 1);

		if (d->kartState == KS_MASK_GRABBED)
		{
			return;
		}

		characterID = data.characterIDs[d->driverID];
		if (characterID == PENTA_PENGUIN)
		{
			characterID = COCO_BANDICOOT;
		}
		if (characterID == FAKE_CRASH)
		{
			characterID = CRASH_BANDICOOT;
		}

		matrixArray = characterID + 7;
		if (characterID == NITROS_OXIDE)
		{
			matrixArray = 7;
		}

		d->matrixArray = matrixArray;
		d->matrixIndex = (u8)inst->animFrame;
		return;
	}

	inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, 1, numFrames - 1);
}
