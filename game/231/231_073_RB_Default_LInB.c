#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b4fe4-0x800b5090.

void RB_Default_LInB(struct Instance *inst)
{
	char *scratch = (char *)0x1f800000;

	int var;

	// high-LOD coll (8 triangles)
	*(s16 *)&scratch[0x13a] = 2;

	*(int *)&scratch[0x13C] = 0x3000;
	*(int *)&scratch[0x140] = 0;
	*(int *)&scratch[0x144] = (int)sdata->gGT->level1->ptr_mesh_info;

	// Make a hitbox
	var = inst->matrix.t[0];
	*(s16 *)&scratch[0x108] = var;
	*(s16 *)&scratch[0x110] = var;
	var = inst->matrix.t[2];
	*(s16 *)&scratch[0x10c] = var;
	*(s16 *)&scratch[0x114] = var;
	var = inst->matrix.t[1];
	*(s16 *)&scratch[0x10a] = var - 0x180;
	*(s16 *)&scratch[0x112] = var + 0x80;

	COLL_SearchBSP_CallbackQUADBLK(
	    (u32 *)&scratch[0x108], (u32 *)&scratch[0x110], (struct ScratchpadStruct *)&scratch[0x118],
	    0); // this scratchpadstruct is +0x118 from 0x1f800000, that may be a problem? all other function calls I've seen just pass 0x1f800000

	RB_MakeInstanceReflective((struct ScratchpadStruct *)&scratch[0x118], inst);
}
