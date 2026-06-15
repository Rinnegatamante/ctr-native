#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 231 0x800b0f1c-0x800b1000.
void RB_RainCloud_FadeAway(struct Thread *t)
{
	struct Instance *inst;
	struct Instance *parentInst;
	struct RainCloud *rcloud;

	inst = t->inst;
	rcloud = t->object;
	parentInst = t->parentThread->inst;

	// offset upward before averaging
	inst->matrix.t[1] += 0x80;

	// X, Y, Z
	for (int i = 0; i < 3; i++)
	{
		// get average between instance and driver
		inst->matrix.t[i] += parentInst->matrix.t[i];
		inst->matrix.t[i] = inst->matrix.t[i] >> 1;
	}

	struct RainLocal *rainLocal = rcloud->rainLocal;
	rainLocal->frameCount -= 2;

	inst->scale[2] += -0x100;
	inst->scale[1] += -0x100;
	inst->scale[0] += -0x100;

	if (inst->scale[0] < 0)
	{
		JitPool_Remove(&sdata->gGT->JitPools.rain, (struct Item *)rainLocal);

		// This thread is now dead
		t->flags |= THREAD_FLAG_DEAD;
	}
	return;
}
