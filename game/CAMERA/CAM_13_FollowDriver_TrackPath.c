#include <common.h>

u32 CAM_FollowDriver_TrackPath(struct CameraDC *cDC, s16 *pos, int speed, int update)
{
	struct CheckpointNode *node = (struct CheckpointNode *)cDC->unk88;
	if (node == NULL)
		return 0;

	pos[0] = node->pos[0];
	pos[1] = node->pos[1] + 0x80;
	pos[2] = node->pos[2];

	if (update)
		cDC->unk94 = 0;

	// TODO(aalhendi): Port full retail path walking/interpolation.
	(void)speed;
	return 0;
}
