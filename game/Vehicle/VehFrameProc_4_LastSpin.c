#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005b5fc-0x8005b6b8
void VehFrameProc_LastSpin(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;
	int targetFrame;
	int numFrames;

	if (inst->animIndex != 0)
	{
		VehFrameProc_Spinning(t, d);
		return;
	}

	numFrames = VehFrameInst_GetNumAnimFrames(inst, 0);
	if (numFrames <= 0)
	{
		return;
	}

	targetFrame = inst->animFrame;

	if (d->turnAngleCurr > 0)
	{
		if (d->unk_LerpToForwards < 0)
		{
			targetFrame = numFrames - 1;
		}
	}

	if ((d->turnAngleCurr < 0) && (d->unk_LerpToForwards > 0))
	{
		targetFrame = 0;
	}

	inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, 3, targetFrame);
}
