#include <common.h>

SVec3 letterLightDir = {0x94F, 0x94F, -0x94F};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b52dc-0x800b5334.

void RB_CtrLetter_ThTick(struct Thread *t)
{
	int sine;
	struct Instance *letterInst;
	struct CtrLetter *letterObj;

	letterInst = t->inst;
	letterObj = t->object;

	// rotate each frame
	letterObj->rot.y += 0x40;
	ConvertRotToMatrix(&letterInst->matrix, &letterObj->rot);

	Vector_SpecLightSpin3D(letterInst, &letterObj->rot.x, &letterLightDir);
}
