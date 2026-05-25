#include <common.h>

// I think this function should return void?

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac4b8-0x800ac5e8.
void RB_Hazard_ThCollide_Generic(struct Thread *thread)
{
	struct Instance *inst;
	struct MineWeapon *mw;

	struct Instance *crateInst;
	struct Thread *crateThread;
	struct Crate *crateObj;

	int modelID;
	int soundID;

	inst = thread->inst;
	mw = thread->object;

	crateInst = mw->crateInst;

	if (crateInst != 0)
	{
		// be careful, dont overwrite local variable
		// "thread", or else you'll kill the wrong thread
		// at the end of the function

		crateObj = (struct Crate *)crateInst->thread->object;

		if (crateObj != 0)
		{
			crateObj->boolPauseCooldown = 0;
		}
	}

	modelID = inst->model->id;

	// if red beaker or green beaker
	if ((u32)(modelID - STATIC_BEAKER_RED) < 2)
	{
		PlaySound3D(0x3f, inst);

		RB_MinePool_Remove(mw);
	}

	else
	{
		// nitro
		if (modelID == PU_EXPLOSIVE_CRATE)
		{
			// shatter sound
			soundID = 0x3f;
		}

		else
		{
			// if not TNT
			if (modelID != STATIC_CRATE_TNT)
			{
				return;
			}

			// at this point, must be TNT

			// if driver hit TNT
			if (mw->driverTarget != 0)
			{
				// quit, explosion handled
				// by TNT thread
				return;
			}

			// if no driver hit TNT,
			// then handle explosion here
			soundID = 0x3d;
		}

		PlaySound3D(soundID, inst);

		RB_MinePool_Remove(mw);

		RB_Explosion_InitGeneric(inst);

		inst->scale[0] = 0;
		inst->scale[1] = 0;
		inst->scale[2] = 0;

		inst->flags |= 0x80;
	}

	// kill thread
	thread->flags |= 0x800;
	return;
}
