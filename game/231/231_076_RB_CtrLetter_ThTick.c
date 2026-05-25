#include <common.h>

s16 letterLightDir[4] = {0x94F, 0x94F, -0x94F, 0};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b52dc-0x800b5334.

void RB_CtrLetter_ThTick(struct Thread *t)
{
	int sine;
	struct Instance *letterInst;
	struct CtrLetter *letterObj;

	letterInst = t->inst;
	letterObj = t->object;

	// rotate each frame
	letterObj->rot[1] += 0x40;
	ConvertRotToMatrix(&letterInst->matrix, &letterObj->rot[0]);

	Vector_SpecLightSpin3D(letterInst, &letterObj->rot[0], &letterLightDir[0]);
}
