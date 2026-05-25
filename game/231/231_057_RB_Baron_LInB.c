#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b37d4-0x800b38dc.

void RB_Baron_LInB(struct Instance *inst)
{
	struct Baron *baronObj;
	struct Thread *t;
	s16 pointIndex;

	if (inst->thread != 0)
		return;

	t = PROC_BirthWithObject(
	    // creation flags
	    SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Baron), NONE, SMALL, STATIC),

	    RB_Baron_ThTick, // behavior
	    "baron",         // debug name
	    0                // thread relative
	);

	if (t == 0)
		return;
	inst->thread = t;
	t->inst = inst;

	inst->scale[0] = 0x1000;
	inst->scale[1] = 0x1000;
	inst->scale[2] = 0x1000;

	baronObj = ((struct Baron *)t->object);
	pointIndex = 1;

	if (inst->name[strlen(inst->name) - 1] == '0')
		pointIndex = sdata->gGT->level1->ptrSpawnType2->numCoords / 2;

	baronObj->pointIndex = pointIndex;
	baronObj->unk1A = 4;
	baronObj->unk22 = 0x18;
	baronObj->unk06 = 0;
	baronObj->otherInst = 0;

	if (inst->model->id == DYNAMIC_VONLABASS)
		inst->flags |= 0x80;

	baronObj->soundID_flags = 0;
}
