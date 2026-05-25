#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae478-0x800ae524.
void RB_MovingExplosive_Explode(struct Thread *t, struct Instance *inst, struct TrackerWeapon *tw)
{
	s16 soundId;
	struct Driver *d;

	// bomb
	if (inst->model->id == DYNAMIC_BOMB)
	{
		// bomb explode
		soundId = 0x49;
		tw->driverParent->instBombThrow = NULL;
	}

	// missile
	else
	{
		d = tw->driverTarget;
		if (d != NULL)
		{
			// remove 2D square-target being drawn on the player's screen
			d->actionsFlagSet &= 0xfbffffff;
		}
		// missile explode
		soundId = 0x4c;
	}

	// play explosion sound
	PlaySound3D(soundId, inst);

	// stop audio of rolling
	OtherFX_RecycleMute(&tw->audioPtr);

	RB_Burst_Init(inst);

	// This thread is now dead
	t->flags |= 0x800;
	return;
}
