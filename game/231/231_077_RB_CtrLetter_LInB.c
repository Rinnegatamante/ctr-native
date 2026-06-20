#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b5334-0x800b53e0.

void RB_CtrLetter_LInB(struct Instance *inst)
{
	struct CtrLetter *letterObj;
	struct Thread *t;

	if (inst->thread == NULL)
	{
		t = PROC_BirthWithObject(
		    // creation flags
		    SIZE_RELATIVE_POOL_BUCKET(sizeof(struct CtrLetter), NONE, SMALL, STATIC),

		    RB_CtrLetter_ThTick, // behavior
		    "ctr",               // debug name
		    0                    // thread relative
		);

		inst->thread = t;
		if (t == 0)
			return;

		t->funcThCollide = (void (*)(struct Thread *))RB_CtrLetter_ThCollide;
		t->inst = inst;

		letterObj = ((struct CtrLetter *)t->object);
		letterObj->rot.x = 0;
		letterObj->rot.y = 0;
		letterObj->rot.z = 0;

		inst->scale.x = 0x1800;
		inst->scale.y = 0x1800;
		inst->scale.z = 0x1800;

		inst->colorRGBA = 0xffc8000;

		// specular light, plus another
		inst->flags |= 0x30000;
	}

	RB_Default_LInB(inst);
}
