#include <common.h>

// NOTE(aalhendi): First 0x18 bytes match `struct BucketSearchParams`; the
// trailing hitDir fields are written by `VehPhysCrash_AnyTwoCars`.
struct VehPhysForce_CollideDrivers_Search
{
	s16 pos[4];
	struct Thread *th;
	int radius;
	s16 dist[4];
	s16 hitDir[4];
};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005ebac-0x8005ee34
void VehPhysForce_CollideDrivers(struct Thread *thread, struct Driver *driver)
{
	u32 stepFlagSet = driver->stepFlagSet;

	driver->velocity.x -= driver->accel.x;
	driver->velocity.y -= driver->accel.y;
	driver->velocity.z -= driver->accel.z;

	if ((stepFlagSet & 0x4000) != 0)
	{
		driver->unkAA |= 1;
	}

	if ((stepFlagSet & 2) != 0)
	{
		VehFire_Increment(driver, 0x78, TURBO_PAD | FREEZE_RESERVES_ON_TURBO_PAD, 0x800);
	}
	else if ((stepFlagSet & 1) != 0)
	{
		if ((sdata->gGT->gameMode2 & CHEAT_TURBOPAD) == 0)
		{
			VehFire_Increment(driver, 0x3c0, TURBO_PAD | FREEZE_RESERVES_ON_TURBO_PAD, 0x100);
		}
		else
		{
			VehFire_Increment(driver, 0x78, TURBO_PAD | FREEZE_RESERVES_ON_TURBO_PAD, 0x800);
		}
	}

	if ((stepFlagSet & 0x8000) != 0)
	{
		thread->inst->vertSplit = 0;
		thread->inst->flags |= 0x2000;
	}
	else
	{
		thread->inst->flags &= ~0x2000;
	}

	if ((thread->flags & 0x1000) == 0)
	{
		struct VehPhysForce_CollideDrivers_Search search;

		search.pos[0] = (s16)(driver->posCurr.x >> 8);
		search.pos[1] = (s16)(driver->posCurr.y >> 8);
		search.pos[2] = (s16)(driver->posCurr.z >> 8);
		search.th = NULL;
		search.radius = 0x7fffffff;

		DECOMP_PROC_CollidePointWithBucket(thread->siblingThread, &search.pos[0]);
		DECOMP_PROC_CollidePointWithBucket(sdata->gGT->threadBuckets[ROBOT].thread, &search.pos[0]);

		if (search.th != NULL)
		{
			int radiusSum = thread->driver_HitRadius + search.th->driver_HitRadius;

			if (search.radius < radiusSum * radiusSum)
			{
				VehPhysCrash_AnyTwoCars(thread, (u16 *)&search, &driver->velocity);
			}
		}
	}

	if ((driver->unkAA & 2) != 0)
	{
		int diffX = (driver->posCurr.x >> 8) - driver->spsHitPos[0];
		int diffZ = (driver->posCurr.z >> 8) - driver->spsHitPos[2];
		int floorDiffY = ((driver->quadBlockHeight >> 8) - driver->spsHitPos[1]) + 4;

		if ((driver->spsNormalVec[0] * diffX + driver->spsNormalVec[1] * floorDiffY + driver->spsNormalVec[2] * diffZ) < 0)
		{
			int diffY = (driver->posCurr.y >> 8) - driver->spsHitPos[1];

			driver->velocity.x += diffX << 6;
			driver->velocity.y += diffY << 6;
			driver->velocity.z += diffZ << 6;
		}
	}
}
