#include <common.h>

static void RB_Blowup_UpdateSlot(int *slot)
{
	struct Instance *inst;
	int nextFrame;

	inst = (struct Instance *)*slot;
	if (inst == NULL)
		return;

	nextFrame = inst->animFrame + 1;
	if (nextFrame < INSTANCE_GetNumAnimFrames(inst, 0))
	{
		inst->animFrame++;
		return;
	}

	INSTANCE_Death(inst);
	*slot = 0;
}

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 231 0x800b17f0-0x800b18f8.
void RB_Blowup_ThTick(struct Thread *t)
{
	int *blowup;
	blowup = t->object;

	RB_Blowup_UpdateSlot(&blowup[1]);
	RB_Blowup_UpdateSlot(&blowup[0]);

	if ((blowup[1] == 0) && (blowup[0] == 0))
		t->flags |= THREAD_FLAG_DEAD;

	ThTick_FastRET(t);
}
