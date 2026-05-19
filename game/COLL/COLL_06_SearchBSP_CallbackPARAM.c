#include <common.h>

static int DECOMP_COLL_SearchBSP_CallbackPARAM_Overlaps(struct BSP *node, short minX, short minY, short minZ, short maxX, short maxY, short maxZ)
{
	return ((node->box.min[1] <= maxY) && (node->box.min[0] <= maxX) && (minX <= node->box.max[0]) && (node->box.min[2] <= maxZ) &&
	        (minZ <= node->box.max[2]) && (minY <= node->box.max[1]));
}

static void DECOMP_COLL_SearchBSP_CallbackPARAM_VisitChild(struct BSP *root, unsigned short childId, short minX, short minY, short minZ, short maxX, short maxY,
                                                           short maxZ, void (*callback)(struct BSP *, struct ScratchpadStruct *),
                                                           struct ScratchpadStruct *param)
{
	struct BSP *child;

	if (childId == 0xffff)
		return;

	child = &root[childId & 0x3fff];
	if (!DECOMP_COLL_SearchBSP_CallbackPARAM_Overlaps(child, minX, minY, minZ, maxX, maxY, maxZ))
		return;

	if ((childId & 0x4000) != 0)
	{
		callback(child, param);
		return;
	}

	DECOMP_COLL_SearchBSP_CallbackPARAM_VisitChild(root, (unsigned short)child->data.branch.childID[1], minX, minY, minZ, maxX, maxY, maxZ, callback, param);

	DECOMP_COLL_SearchBSP_CallbackPARAM_VisitChild(root, (unsigned short)child->data.branch.childID[0], minX, minY, minZ, maxX, maxY, maxZ, callback, param);
}

void DECOMP_COLL_SearchBSP_CallbackPARAM(struct BSP *root, struct BoundingBox *bbox, void (*callback)(struct BSP *, struct ScratchpadStruct *),
                                         struct ScratchpadStruct *param)
{
	if (root == NULL)
		return;

	short minX = bbox->min[0];
	short minY = bbox->min[1];
	short minZ = bbox->min[2];
	short maxX = bbox->max[0];
	short maxY = bbox->max[1];
	short maxZ = bbox->max[2];

	DECOMP_COLL_SearchBSP_CallbackPARAM_VisitChild(root, (unsigned short)root->data.branch.childID[1], minX, minY, minZ, maxX, maxY, maxZ, callback, param);

	DECOMP_COLL_SearchBSP_CallbackPARAM_VisitChild(root, (unsigned short)root->data.branch.childID[0], minX, minY, minZ, maxX, maxY, maxZ, callback, param);
}
