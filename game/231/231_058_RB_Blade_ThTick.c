#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 231 0x800b38e4-0x800b3978.
void RB_Blade_ThTick(struct Thread *t)
{
	struct Blade *bladeObj;
	struct Instance *bladeInst;
	SVec3 rot;

	bladeObj = (struct Blade *)t->object;
	bladeInst = t->inst;

	rot.x = bladeInst->instDef->rot.x;
	rot.y = bladeInst->instDef->rot.y + 0x400;
	rot.z = bladeObj->angle;

	bladeObj->angle += 0x100;

	// converted to TEST in rebuildPS1
	ConvertRotToMatrix(&bladeInst->matrix, rot.v);

	bladeInst->scale.x = 0x1000;
	bladeInst->scale.y = 0x1000;
	bladeInst->scale.z = 0x1000;

	ThTick_FastRET(t);
}
