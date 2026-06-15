#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac638-0x800ac6b4.
void RB_Potion_OnShatter_TeethSearch(struct Instance *inst)
{
	struct ScratchpadStruct *sps = CTR_SCRATCHPAD_PTR(struct ScratchpadStruct, 0x108);

	sps->Input1.pos.x = (s16)inst->matrix.t[0];
	sps->Input1.pos.y = (s16)inst->matrix.t[1];
	sps->Input1.pos.z = (s16)inst->matrix.t[2];
	sps->Input1.hitRadius = 0x140;
	sps->Input1.hitRadiusSquared = 0x19000;
	sps->Input1.modelID = inst->model->id;

	sps->Union.ThBuckColl.thread = inst->thread;
	sps->Union.ThBuckColl.funcCallback = RB_Potion_OnShatter_TeethCallback;

	PROC_StartSearch_Self(sps);
}
