#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005b510-0x8005b5fc
void VehFrameProc_Spinning(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;
	int numFrames = VehFrameInst_GetNumAnimFrames(inst, inst->animIndex);
	int targetFrame;

	if (numFrames <= 0)
	{
		return;
	}

	if (inst->animIndex != 0)
	{
		targetFrame = VehFrameInst_GetStartFrame(inst->animIndex, numFrames);

		if ((u32)(inst->animIndex - 2) < 2)
		{
			d->matrixArray = 0;
			d->matrixIndex = 0;
		}

		if (inst->animFrame == targetFrame)
		{
			numFrames = VehFrameInst_GetNumAnimFrames(inst, 0);
			if (numFrames <= 0)
			{
				return;
			}

			inst->animIndex = 0;
		}

		if (inst->animIndex != 0)
		{
			inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, 4, targetFrame);
			return;
		}
	}

	targetFrame = 0;
	if (d->KartStates.Spinning.spinDir >= 0)
	{
		targetFrame = numFrames - 1;
	}

	inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, 4, targetFrame);
}
