#include <common.h>

static s32 DECOMP_PROC_CollideHitbox_MipsSquare(s32 value)
{
	return (s32)(u32)((s64)value * (s64)value);
}

void DECOMP_PROC_CollideHitboxWithBucket(struct Thread *collThread, struct ScratchpadStruct *sps, struct Thread *ignoredThread)
{
	s32 distX;
	s32 distY;
	s32 distZ;
	s32 dist;
	struct Instance *inst;
	void (*callback)(struct ScratchpadStruct *, void *);

	for (/**/; collThread != NULL; collThread = collThread->siblingThread)
	{
		if (collThread->childThread != NULL)
			DECOMP_PROC_CollideHitboxWithBucket(collThread->childThread, sps, ignoredThread);

		if (collThread == ignoredThread)
			continue;

		if ((collThread->flags & 0x1800) != 0)
			continue;

		inst = collThread->inst;

		distX = (int)sps->Input1.pos[0] - inst->matrix.t[0];
		distY = (int)sps->Input1.pos[1] - inst->matrix.t[1];
		distZ = (int)sps->Input1.pos[2] - inst->matrix.t[2];

		dist = DECOMP_PROC_CollideHitbox_MipsSquare(distX);
		if (dist > 0x0fffffff)
			continue;

		s32 distYSquared = DECOMP_PROC_CollideHitbox_MipsSquare(distY);
		dist += distYSquared;
		if (distYSquared > 0x0fffffff)
			continue;

		s32 distZSquared = DECOMP_PROC_CollideHitbox_MipsSquare(distZ);
		dist += distZSquared;
		if (distZSquared > 0x0fffffff)
			continue;

		if (dist >= sps->Input1.hitRadiusSquared)
			continue;

		sps->Union.ThBuckColl.distance[0] = distX;
		sps->Union.ThBuckColl.distance[1] = distY;
		sps->Union.ThBuckColl.distance[2] = distZ;

		callback = (void (*)(struct ScratchpadStruct *, void *))sps->Union.ThBuckColl.funcCallback;
		callback(sps, collThread);
	}
}
