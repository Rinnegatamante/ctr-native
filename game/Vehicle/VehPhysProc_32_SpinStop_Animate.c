#include <common.h>

// only Animate is needed, see StopSpin_Init for details

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800643d4-0x800644d0
void VehPhysProc_SpinStop_Animate(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;

	int numFrames = VehFrameInst_GetNumAnimFrames(inst, inst->animIndex);

	if (numFrames > 0)
	{
		// steer from left to right, to exaggerate the force when steering stops abruptly
		if (d->KartStates.Spinning.spinDir == -1)
		{
			inst->animFrame += 5;

			if (inst->animFrame < numFrames)
				return;

			inst->animFrame = numFrames - 1;
			d->KartStates.Spinning.spinDir = 0;
			return;
		}

		// steer from right to left, to exaggerate the force when steering stops abruptly
		if (d->KartStates.Spinning.spinDir == 1)
		{
			inst->animFrame -= 5;

			if (inst->animFrame >= 0)
				return;

			inst->animFrame = 0;
			d->KartStates.Spinning.spinDir = 0;
			return;
		}

		int targetFrame = VehFrameInst_GetStartFrame(0, numFrames);
		inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, 2, targetFrame);

		if (inst->animFrame != targetFrame)
			return;
	}

	d->funcPtrs[0] = DECOMP_VehPhysProc_Driving_Init;
}

void DECOMP_VehPhysProc_SpinStop_Animate(struct Thread *t, struct Driver *d)
{
	VehPhysProc_SpinStop_Animate(t, d);
}
