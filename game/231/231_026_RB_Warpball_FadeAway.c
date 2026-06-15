#include <common.h>

static const s16 s_warpballFadeScale[6 * 3] = {
    4505, 5120, 4096, 5226, 8192, 4096, 5600, 5501, 4096, 5272, 3183, 4096, 4242, 1411, 3337, 2878, 437, 1668,
};

static const s32 s_warpballFadeY[6] = {
    -64, -256, -87, 57, 167, 228,
};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae524-0x800ae604.
// NOTE(aalhendi): Native uses retail fade scale/Y table bytes from 0x800b2c88 and 0x800b2cac.
void RB_Warpball_FadeAway(struct Thread *t)
{
	s16 frameId;
	int iVar2;
	struct TrackerWeapon *tw;
	struct Instance *inst;
	struct Driver *d;
	struct GameTracker *gGT;

	gGT = sdata->gGT;

	tw = t->object;
	inst = t->inst;
	frameId = tw->fadeAway_frameCount5;

	if (frameId > 5)
	{
		d = tw->driverTarget;

		if (d != NULL)
		{
			// remove 2D square-target being drawn on the player's screen
			d->actionsFlagSet &= ~ACTION_TRACKER_TARGETED;
		}

		// remove active warpball flag
		gGT->gameMode1 &= ~(WARPBALL_HELD);

		// This thread is now dead
		t->flags |= THREAD_FLAG_DEAD;
		return;
	}

	// set scale (x, y, z)
	inst->scale[0] = s_warpballFadeScale[(frameId * 3) + 0];
	inst->scale[1] = s_warpballFadeScale[(frameId * 3) + 1];
	inst->scale[2] = s_warpballFadeScale[(frameId * 3) + 2];

	inst->matrix.t[1] = tw->distFromGround + s_warpballFadeY[frameId];

	tw->fadeAway_frameCount5 += 1;

	return;
}
