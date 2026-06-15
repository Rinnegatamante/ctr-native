#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 231 0x800ae668-0x800ae778.
struct CheckpointNode *RB_Warpball_NewPathNode(struct CheckpointNode *cn, struct Driver *d)
{
	struct GameTracker *gGT = sdata->gGT;
	u8 pathIndex;
	u8 targetIndex;
	int foundLeftPath;

	if (d == NULL)
	{
		return &gGT->level1->ptr_restart_points[cn->nextIndex_forward];
	}

	foundLeftPath = 0;
	targetIndex = d->checkpoint.branchChoiceIndex;
	pathIndex = cn->nextIndex_left;

	if (targetIndex == pathIndex)
	{
		return &gGT->level1->ptr_restart_points[pathIndex];
	}

	if (pathIndex != 0xff)
	{
		struct CheckpointNode *currNode = cn;

		for (int i = 0; i < 3; i++)
		{
			if (currNode->nextIndex_left == 0xff)
			{
				pathIndex = currNode->nextIndex_forward;
			}
			else
			{
				pathIndex = currNode->nextIndex_left;
			}

			currNode = &gGT->level1->ptr_restart_points[pathIndex];

			if (targetIndex == currNode->nextIndex_forward)
			{
				foundLeftPath = 1;
				break;
			}
		}
	}

	if (foundLeftPath)
	{
		return &gGT->level1->ptr_restart_points[cn->nextIndex_left];
	}

	return &gGT->level1->ptr_restart_points[cn->nextIndex_forward];
}
