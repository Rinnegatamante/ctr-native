#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ad92c-0x800ad9ac.
void RB_Explosion_ThTick(struct Thread *t)
{
	struct Instance *inst = t->inst;

	int frame = inst->animFrame;
	int total = INSTANCE_GetNumAnimFrames(inst, 0);

	if ((frame + 1) < total)
	{
		inst->animFrame++;
		return;
	}

	// dead thread
	t->flags |= 0x800;
}
