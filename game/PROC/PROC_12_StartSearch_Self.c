#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80042544-0x800425d4
void DECOMP_PROC_StartSearch_Self(struct ScratchpadStruct *sps)
{
	s16 hitRadius;
	struct GameTracker *gGT;

	hitRadius = sps->Input1.hitRadius;

	sps->Union.ThBuckColl.min[0] = (s16)((u16)sps->Input1.pos[0] - (u16)hitRadius);
	sps->Union.ThBuckColl.min[1] = (s16)((u16)sps->Input1.pos[1] - (u16)hitRadius);
	sps->Union.ThBuckColl.min[2] = (s16)((u16)sps->Input1.pos[2] - (u16)hitRadius);

	sps->Union.ThBuckColl.max[0] = (s16)((u16)sps->Input1.pos[0] + (u16)hitRadius);
	sps->Union.ThBuckColl.max[1] = (s16)((u16)sps->Input1.pos[1] + (u16)hitRadius);
	sps->Union.ThBuckColl.max[2] = (s16)((u16)sps->Input1.pos[2] + (u16)hitRadius);

	gGT = sdata->gGT;

	COLL_SearchBSP_CallbackPARAM(gGT->level1->ptr_mesh_info->bspRoot, (struct BoundingBox *)&sps->Union.ThBuckColl.min[0],
	                             DECOMP_PROC_PerBspLeaf_CheckInstances, sps);
}
