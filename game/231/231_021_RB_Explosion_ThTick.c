#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 231 0x800ad92c-0x800ad9ac.
void RB_Explosion_ThTick(struct Thread *t)
{
	struct Instance *inst = t->inst;

	int frame = inst->animFrame;
	int total = INSTANCE_GetNumAnimFrames(inst, 0);

	if ((frame + 1) < total)
	{
		inst->animFrame++;
	}
	else
	{
		// dead thread
		t->flags |= THREAD_FLAG_DEAD;
	}

	ThTick_FastRET(t);
}
