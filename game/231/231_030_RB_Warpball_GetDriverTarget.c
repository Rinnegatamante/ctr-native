#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae7dc-0x800aeaac.
struct Driver *RB_Warpball_GetDriverTarget(struct TrackerWeapon *tw, struct Instance *inst)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *bestDriver = NULL;

	if ((tw->flags & 1) == 0)
	{
		for (int i = 0; i < 8; i++)
		{
			struct Driver *driver = gGT->driversInRaceOrder[i];

			if ((driver != NULL) && (driver != tw->driverParent) && ((driver->actionsFlagSet & ACTION_RACE_FINISHED) == 0))
			{
				return driver;
			}
		}

		return bestDriver;
	}

	struct CheckpointNode *nodes = gGT->level1->ptr_restart_points;
	struct CheckpointNode *node1 = &nodes[tw->ptrNodeCurr->nextIndex_forward];
	struct CheckpointNode *node2 = &nodes[node1->nextIndex_forward];
	int trackDistance = nodes[0].distToFinish << 3;
	SVec3 pathVector;
	SVec3 orbVector;
	s32 projectedDistance;
	int bestDistance = 0x7fffffff;

	pathVector.x = (s16)(node1->pos[0] - node2->pos[0]);
	pathVector.y = (s16)(node1->pos[1] - node2->pos[1]);
	pathVector.z = (s16)(node1->pos[2] - node2->pos[2]);
	MATH_VectorNormalize(&pathVector);

	orbVector.x = (s16)(inst->matrix.t[0] - node1->pos[0]);
	orbVector.y = (s16)(inst->matrix.t[1] - node1->pos[1]);
	orbVector.z = (s16)(inst->matrix.t[2] - node1->pos[2]);

	// NOTE(aalhendi): Retail uses GTE MVMVA MAC1; this is the same row0 dot product.
	projectedDistance = ((s32)pathVector.x * orbVector.x) + ((s32)pathVector.y * orbVector.y) + ((s32)pathVector.z * orbVector.z);

	projectedDistance = ((node1->distToFinish << 3) + (projectedDistance >> 12) + 0x200) % trackDistance;

	for (int i = 0; i < 8; i++)
	{
		struct Driver *driver = gGT->drivers[i];

		if ((driver != NULL) && ((tw->driversHit & (1u << (i & 0x1f))) == 0) && ((driver->actionsFlagSet & ACTION_RACE_FINISHED) == 0) &&
		    (driver->kartState != KS_MASK_GRABBED))
		{
			int distance = projectedDistance - driver->distanceToFinish_curr;

			if (distance < 0)
			{
				distance += trackDistance;
			}

			if (distance < bestDistance)
			{
				bestDistance = distance;
				bestDriver = driver;
			}
		}
	}

	return bestDriver;
}
