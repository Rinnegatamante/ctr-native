#include <common.h>

static int COLL_SearchBSP_CallbackPARAM_Overlaps(struct BSP *node, s16 minX, s16 minY, s16 minZ, s16 maxX, s16 maxY, s16 maxZ)
{
	return ((node->box.min[1] <= maxY) && (node->box.min[0] <= maxX) && (minX <= node->box.max[0]) && (node->box.min[2] <= maxZ) &&
	        (minZ <= node->box.max[2]) && (minY <= node->box.max[1]));
}

static void COLL_SearchBSP_CallbackPARAM_PushChild(struct BSP *root, u16 childId, s16 minX, s16 minY, s16 minZ, s16 maxX, s16 maxY, s16 maxZ, u16 **stackTop)
{
	struct BSP *child;

	if (childId == 0xffff)
		return;

	child = &root[childId & 0x3fff];
	if (!COLL_SearchBSP_CallbackPARAM_Overlaps(child, minX, minY, minZ, maxX, maxY, maxZ))
		return;

	**stackTop = childId;
	(*stackTop)++;
}

static void COLL_SearchBSP_CallbackPARAM_PushChildren(struct BSP *root, struct BSP *node, s16 minX, s16 minY, s16 minZ, s16 maxX, s16 maxY, s16 maxZ,
                                                      u16 **stackTop)
{
	// Retail pushes child 0 then child 1; the scratchpad stack pops child 1 first.
	COLL_SearchBSP_CallbackPARAM_PushChild(root, (u16)node->data.branch.childID[0], minX, minY, minZ, maxX, maxY, maxZ, stackTop);
	COLL_SearchBSP_CallbackPARAM_PushChild(root, (u16)node->data.branch.childID[1], minX, minY, minZ, maxX, maxY, maxZ, stackTop);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8001ebec-0x8001ede4
void COLL_SearchBSP_CallbackPARAM(struct BSP *root, struct BoundingBox *bbox, void (*callback)(struct BSP *, struct ScratchpadStruct *),
                                  struct ScratchpadStruct *param)
{
	u16 *stackBase;
	u16 *stackTop;

	if (root == NULL)
		return;

	s16 minX = bbox->min[0];
	s16 minY = bbox->min[1];
	s16 minZ = bbox->min[2];
	s16 maxX = bbox->max[0];
	s16 maxY = bbox->max[1];
	s16 maxZ = bbox->max[2];

	// Retail stores pending child IDs at scratchpad 0x1f800070 and pops them
	// LIFO, preserving the original BSP traversal order without host recursion.
	stackBase = CTR_SCRATCHPAD_PTR(u16, 0x70);
	stackTop = stackBase;

	COLL_SearchBSP_CallbackPARAM_PushChildren(root, root, minX, minY, minZ, maxX, maxY, maxZ, &stackTop);

	while (stackTop != stackBase)
	{
		u16 childId;
		struct BSP *child;

		stackTop--;
		childId = *stackTop;
		child = &root[childId & 0x3fff];

		if ((childId & 0x4000) != 0)
		{
			callback(child, param);
			continue;
		}

		COLL_SearchBSP_CallbackPARAM_PushChildren(root, child, minX, minY, minZ, maxX, maxY, maxZ, &stackTop);
	}
}
