#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8001eb0c-0x8001ebec
void COLL_SearchBSP_CallbackQUADBLK(u32 *posTop, u32 *posBottom, struct ScratchpadStruct *sps, int hitRadius)
{
	u32 topXY;
	u32 bottomXY;
	s16 topZ;
	s16 bottomZ;
	s16 topX;
	s16 topY;
	s16 bottomX;
	s16 bottomY;
	s16 min;
	s16 max;
	int hitRadiusSquared;
	struct mesh_info *meshInfo;

	hitRadiusSquared = hitRadius * hitRadius;

	sps->Input1.hitRadius = hitRadius;
	sps->Union.QuadBlockColl.hitRadius = hitRadius;
	sps->boolDidTouchHitbox = 0;
	sps->unk3C = 0;
	sps->boolDidTouchQuadblock = 0;
	*(int *)&sps->dataOutput[0] = 0;
	meshInfo = sps->ptr_mesh_info;
	sps->numInstHitboxesHit = 0;

	topXY = posTop[0];
	topZ = *(s16 *)(posTop + 1);

	sps->Input1.pos[0] = (s16)topXY;
	sps->Input1.pos[1] = (s16)(topXY >> 16);
	sps->Input1.pos[2] = topZ;
	sps->Union.QuadBlockColl.hitPos[0] = (s16)topXY;
	sps->Union.QuadBlockColl.hitPos[1] = (s16)(topXY >> 16);
	sps->Union.QuadBlockColl.hitPos[2] = topZ;

	bottomXY = posBottom[0];
	bottomZ = *(s16 *)(posBottom + 1);

	sps->Union.QuadBlockColl.pos[0] = (s16)bottomXY;
	sps->Union.QuadBlockColl.pos[1] = (s16)(bottomXY >> 16);
	sps->Union.QuadBlockColl.pos[2] = bottomZ;

	sps->countByOne_ForWhatReason = 0x1000;
	sps->Input1.hitRadiusSquared = hitRadiusSquared;
	sps->Union.QuadBlockColl.hitRadiusSquared = hitRadiusSquared;

	topX = sps->Input1.pos[0];
	topY = sps->Input1.pos[1];
	topZ = sps->Input1.pos[2];
	bottomX = sps->Union.QuadBlockColl.pos[0];
	bottomY = sps->Union.QuadBlockColl.pos[1];
	bottomZ = sps->Union.QuadBlockColl.pos[2];

	min = bottomX;
	max = topX;
	if ((topX - bottomX) < 0)
	{
		min = topX;
		max = bottomX;
	}
	sps->bbox.min[0] = min;
	sps->bbox.max[0] = max;

	min = bottomY;
	max = topY;
	if ((topY - bottomY) < 0)
	{
		min = topY;
		max = bottomY;
	}
	sps->bbox.min[1] = min;
	sps->bbox.max[1] = max;

	min = bottomZ;
	max = topZ;
	if ((topZ - bottomZ) < 0)
	{
		min = topZ;
		max = bottomZ;
	}
	sps->bbox.min[2] = min;
	sps->bbox.max[2] = max;

	COLL_SearchBSP_CallbackPARAM(meshInfo->bspRoot, &sps->bbox, COLL_FIXED_BSPLEAF_TestQuadblocks, sps);
}
