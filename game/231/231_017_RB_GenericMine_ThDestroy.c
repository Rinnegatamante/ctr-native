#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ad250-0x800ad310.
void RB_GenericMine_ThDestroy(struct Thread *t, struct Instance *inst, struct MineWeapon *mw)
{
	u32 model;
	u16 param;

	model = inst->model->id;

	if (model == PU_EXPLOSIVE_CRATE)
	{
		// glass shatter
		param = 0x3f;

		PlaySound3D(param, inst);

		RB_Blowup_Init(inst);
	}
	else if (model == STATIC_CRATE_TNT)
	{
		// tnt explosion sound
		param = 0x3d;

		PlaySound3D(param, inst);

		RB_Blowup_Init(inst);
	}
	else
	{
		// play sound of glass shatter
		PlaySound3D(0x3f, inst);

		RB_Explosion_InitPotion(inst);
	}

	// Set scale (x, y, z) to zero
	inst->scale[0] = 0;
	inst->scale[1] = 0;
	inst->scale[2] = 0;

	// make invisible
	inst->flags |= 0x80;

	RB_MinePool_Remove(mw);

	// this thread is now dead
	t->flags |= 0x800;

	return;
}
