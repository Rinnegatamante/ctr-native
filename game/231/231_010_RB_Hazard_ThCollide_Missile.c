#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac42c-0x800ac4b8.
// NOTE(aalhendi): Native ThCollide ABI is void; retail returns v0=1.
void RB_Hazard_ThCollide_Missile(struct Thread *thread)
{
	struct TrackerWeapon *tw;
	struct Driver *driver;
	struct Instance *inst;

	inst = thread->inst;
	tw = inst->thread->object;

	// could I also just do thread->modelID?
	if (inst->model->id == DYNAMIC_ROCKET)
	{
		// get driver
		driver = tw->driverTarget;

		// if driver is valid
		if (driver != 0)
		{
			// remove 2D square-target being drawn on the player's screen
			driver->actionsFlagSet &= 0xfbffffff;
		}

		// play audio of explosion
		PlaySound3D(0x4c, inst);

		// stop audio of moving
		OtherFX_RecycleMute(&tw->audioPtr);

		// kill thread
		thread->flags |= 0x800;
	}

	return;
}
