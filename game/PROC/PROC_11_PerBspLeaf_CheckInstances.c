#include <common.h>

static s32 DECOMP_PROC_PerBspLeaf_MipsSquare(s32 value)
{
	return (s32)(u32)((s64)value * (s64)value);
}

void DECOMP_PROC_PerBspLeaf_CheckInstances(struct BSP *bspLeaf, struct ScratchpadStruct *sps)
{
	s32 distX;
	s32 distY;
	s32 distZ;
	s32 dist;
	struct BSP *bspHitbox;
	struct InstDef *instDef;
	void (*callback)(struct ScratchpadStruct *, void *);

	bspHitbox = bspLeaf->data.leaf.bspHitboxArray;
	if (bspHitbox == NULL)
		return;

	if (*(int *)bspHitbox == 0)
		return;

	for (/**/; *(int *)bspHitbox != 0; bspHitbox++)
	{
		if ((bspHitbox->flag & 0x80) == 0)
			continue;

		instDef = bspHitbox->data.hitbox.instDef;
		if ((instDef != NULL) && ((instDef->ptrInstance->flags & 0xf) == 0))
			continue;

		distX = (int)sps->Input1.pos[0] - (int)bspHitbox->data.hitbox.unkShort[0];
		distY = (int)sps->Input1.pos[1] - (int)bspHitbox->data.hitbox.unkShort[1];
		distZ = (int)sps->Input1.pos[2] - (int)bspHitbox->data.hitbox.unkShort[2];

		dist = DECOMP_PROC_PerBspLeaf_MipsSquare(distX);
		if (dist > 0x0fffffff)
			continue;

		s32 distYSquared = DECOMP_PROC_PerBspLeaf_MipsSquare(distY);
		dist += distYSquared;
		if (distYSquared > 0x0fffffff)
			continue;

		s32 distZSquared = DECOMP_PROC_PerBspLeaf_MipsSquare(distZ);
		dist += distZSquared;
		if (distZSquared > 0x0fffffff)
			continue;

		if (dist >= sps->Input1.hitRadiusSquared)
			continue;

		sps->Union.ThBuckColl.distance[0] = distX;
		sps->Union.ThBuckColl.distance[1] = distY;
		sps->Union.ThBuckColl.distance[2] = distZ;

		callback = (void (*)(struct ScratchpadStruct *, void *))sps->Union.ThBuckColl.funcCallback;
		callback(sps, bspHitbox);
	}
}
