#include <common.h>

static char s_cloud1[] = "cloud1";

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 231 0x800b1220-0x800b1458.
void RB_RainCloud_Init(struct Driver *d)
{
	struct Instance *cloudInst;
	struct RainCloud *rcloud;
	struct RainLocal *rlocal;

	// if driver -> cloudTh is invalid
	if (d->thCloud == NULL)
	{
		cloudInst = INSTANCE_BirthWithThread(0x42, s_cloud1, SMALL, OTHER, RB_RainCloud_ThTick, sizeof(struct RainCloud), d->instSelf->thread);

		cloudInst->thread->funcThDestroy = PROC_DestroyInstance;

		*(int *)&cloudInst->matrix.m[0][0] = 0x1000;
		*(int *)&cloudInst->matrix.m[0][2] = 0;
		*(int *)&cloudInst->matrix.m[1][1] = 0x1000;
		*(int *)&cloudInst->matrix.m[2][0] = 0;
		cloudInst->matrix.m[2][2] = 0x1000;

		// cloud->posX = driver->posX
		cloudInst->matrix.t[0] = d->instSelf->matrix.t[0];
		cloudInst->matrix.t[1] = d->instSelf->matrix.t[1] + 0x80;
		cloudInst->matrix.t[2] = d->instSelf->matrix.t[2];

		cloudInst->alphaScale = 0x800;

		cloudInst->depthBiasNormal = d->instSelf->depthBiasNormal;
		cloudInst->depthBiasSecondary = d->instSelf->depthBiasSecondary;

		// add rain to pool
		rlocal = (struct RainLocal *)JitPool_Add(&sdata->gGT->JitPools.rain);

		if (rlocal != NULL)
		{
			rlocal->frameCount = 0x1e;

			rlocal->unk1[0] = 0;
			rlocal->unk1[1] = 0;
			rlocal->unk1[2] = 0;

			rlocal->vel.x = 0;
			rlocal->vel.y = -0x28;
			rlocal->vel.z = 0;

			rlocal->pos.x = d->instSelf->matrix.t[0];
			rlocal->pos.y = d->instSelf->matrix.t[1] + 0x80;
			rlocal->pos.z = d->instSelf->matrix.t[2];

			rlocal->cloudInst = cloudInst;
		}

		rcloud = cloudInst->thread->object;
		rcloud->timeMS = 0x1e00; // 7.68s
		rcloud->rainLocal = rlocal;
		rcloud->effect = RAIN_CLOUD_EFFECT_ITEM_ROLL;

		if (
		    // if driver has no weapon
		    (d->heldItemID == 0xf) ||

		    (d->noItemTimer != 0))
		{
			rcloud->effect = RAIN_CLOUD_EFFECT_SLOW;
		}

		d->thCloud = cloudInst->thread;
	}

	// if cloud already exists, and
	// driver hits another red potion
	else
	{
		rcloud = d->thCloud->object;

		// set duration to 8 seconds
		rcloud->timeMS = 0x1e00;

		// random number
		int rng = MixRNG_Scramble();

		rcloud->effect = (RainCloudEffect)((rng % 400) / 100);
	}
	return;
}
