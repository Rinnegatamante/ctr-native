#include <common.h>

static void VehStuckProc_MaskGrab_SearchBsp(struct Driver *d, struct ScratchpadStruct *sps)
{
	struct GameTracker *gGT = sdata->gGT;
	s16 topX = d->posCurr.x >> 8;
	s16 topY = d->posCurr.y >> 8;
	s16 topZ = d->posCurr.z >> 8;
	s16 bottomY = topY - 0x100;

	sps->Input1.pos[0] = topX;
	sps->Input1.pos[1] = bottomY;
	sps->Input1.pos[2] = topZ;

	sps->Union.QuadBlockColl.pos[0] = topX;
	sps->Union.QuadBlockColl.pos[1] = topY;
	sps->Union.QuadBlockColl.pos[2] = topZ;

	sps->Union.QuadBlockColl.searchFlags = 0;
	if (gGT->numPlyrCurrGame < 3)
	{
		sps->Union.QuadBlockColl.searchFlags = 2;
	}

	sps->boolDidTouchQuadblock = 0;
	sps->unk3C = 0;
	sps->countByOne_ForWhatReason = 0x1000;
	*(u32 *)&sps->dataOutput[0] = 0;

	sps->bbox.min[0] = topX;
	sps->bbox.max[0] = topX;
	sps->bbox.min[1] = (bottomY < topY) ? bottomY : topY;
	sps->bbox.max[1] = (topY < bottomY) ? bottomY : topY;
	sps->bbox.min[2] = topZ;
	sps->bbox.max[2] = topZ;

	sps->Union.QuadBlockColl.hitPos[0] = sps->Input1.pos[0];
	sps->Union.QuadBlockColl.hitPos[1] = sps->Input1.pos[1];
	sps->Union.QuadBlockColl.hitPos[2] = sps->Input1.pos[2];

	COLL_SearchBSP_CallbackPARAM(sps->ptr_mesh_info->bspRoot, &sps->bbox, COLL_FIXED_BSPLEAF_TestQuadblocks, sps);
}

void VehStuckProc_MaskGrab_FindDestPos(struct Driver *d, struct QuadBlock *quad)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Level *level = gGT->level1;
	struct mesh_info *mesh = level->ptr_mesh_info;

	if ((level->cnt_restart_points < 1) || (level->ptr_restart_points == NULL) || (quad->checkpointIndex == -1))
	{
		struct LevVertex *verts = mesh->ptrVertexArray;
		struct LevVertex *v0 = &verts[quad->index[0]];
		struct LevVertex *v3 = &verts[quad->index[3]];

		d->posCurr.x = (v0->pos[0] + v3->pos[0]) * 0x80;
		d->posCurr.y = (v0->pos[1] + v3->pos[1] + 0x80) * 0x80;
		d->posCurr.z = (v0->pos[2] + v3->pos[2]) * 0x80;
	}
	else
	{
		struct ScratchpadStruct *sps = CTR_SCRATCHPAD_PTR(struct ScratchpadStruct, 0x108);
		struct Thread *driverThread = d->instSelf->thread;
		struct CheckpointNode *respawn = &level->ptr_restart_points[(u8)quad->checkpointIndex];
		struct CheckpointNode *nextRespawn;

		sps->Input1.hitRadius = driverThread->driver_HitRadius;
		sps->Input1.hitRadiusSquared = driverThread->driver_unk1;
		sps->Union.QuadBlockColl.hitRadius = driverThread->driver_HitRadius;
		sps->Union.QuadBlockColl.hitRadiusSquared = driverThread->driver_unk1;
		sps->ptr_mesh_info = mesh;
		sps->Union.QuadBlockColl.qbFlagsIgnored = 0x4010;
		sps->Union.QuadBlockColl.qbFlagsWanted = 0x1000;
		d->distanceDrivenBackwards = 0;

		do
		{
			do
			{
				nextRespawn = &level->ptr_restart_points[respawn->nextIndex_forward];

				d->posCurr.x = respawn->pos[0] << 8;
				d->posCurr.y = (respawn->pos[1] + 0x80) << 8;
				d->posCurr.z = respawn->pos[2] << 8;

				d->rotCurr.x = 0;
				d->rotCurr.y = ratan2(nextRespawn->pos[0] - respawn->pos[0], nextRespawn->pos[2] - respawn->pos[2]);
				d->rotCurr.z = 0;

				VehStuckProc_MaskGrab_SearchBsp(d, sps);
				respawn = nextRespawn;
			} while ((sps->boolDidTouchQuadblock == 0) || ((*(u32 *)&sps->dataOutput[0] & 0x4000) != 0));

			struct Thread *playerThread = gGT->threadBuckets[PLAYER].thread;
			while (playerThread != NULL)
			{
				struct Driver *other = playerThread->object;

				if (other != d)
				{
					int diffX = d->posCurr.x - other->posCurr.x;
					int diffZ = d->posCurr.z - other->posCurr.z;

					if (diffX < 0)
						diffX = -diffX;

					if (diffX < 0x2000)
						break;

					if (diffZ < 0)
						diffZ = -diffZ;

					if (diffZ < 0x2000)
						break;
				}

				playerThread = playerThread->siblingThread;
			}

			if (playerThread == NULL)
				break;
		} while (1);
	}

	gGT->cameraDC[d->driverID].flags |= 1;
}
