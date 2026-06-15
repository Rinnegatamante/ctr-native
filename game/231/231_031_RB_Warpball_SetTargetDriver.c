#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 231 0x800aeaac-0x800aece0.
void RB_Warpball_SetTargetDriver(struct TrackerWeapon *tw)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *target = tw->driverTarget;

	if (target == NULL)
	{
		return;
	}

	struct CheckpointNode *nodes = gGT->level1->ptr_restart_points;
	struct CheckpointNode *targetNode = &nodes[target->checkpoint.currentIndex];
	struct CheckpointNode *prevNode = targetNode;
	int targetDistance = target->distanceToFinish_curr;

	while ((((int)targetNode->distToFinish << 3) >= targetDistance) && (targetNode != nodes))
	{
		prevNode = targetNode;
		targetNode = RB_Warpball_NewPathNode(targetNode, tw->driverTarget);
	}
	targetNode = prevNode;

	struct CheckpointNode *rightPathNode = NULL;

	if ((tw->flags & 4) == 0)
	{
		struct CheckpointNode *pathStarts[2];

		pathStarts[0] = tw->ptrNodeCurr;
		pathStarts[1] = rightPathNode;

		for (int i = 0; (i < 2) && ((tw->flags & 4) == 0); i++)
		{
			struct CheckpointNode *pathNode = pathStarts[i];

			if (pathNode == NULL)
			{
				continue;
			}

			for (int j = 0; j < 3; j++)
			{
				if (pathNode == targetNode)
				{
					tw->flags = (tw->flags & ~8) | 4;
					break;
				}

				if (pathNode->nextIndex_right != 0xff)
				{
					rightPathNode = &nodes[pathNode->nextIndex_right];
					pathStarts[1] = rightPathNode;
				}

				pathNode = &nodes[pathNode->nextIndex_backward];
			}
		}
	}

	struct CheckpointNode *pathNode = tw->ptrNodeCurr;

	for (int i = 0; i < 3; i++)
	{
		if (pathNode == targetNode)
		{
			tw->flags = (tw->flags & ~8) | 4;
			return;
		}

		pathNode = RB_Warpball_NewPathNode(pathNode, tw->driverTarget);
	}
}
