#include <common.h>

void CAM_FollowDriver_AngleAxis(struct CameraDC *cDC, struct Driver *d, int scratchpad, s16 *pushBufferPos, s16 *pushBufferRot)
{
	// TODO(aalhendi): Port the terrain axis-angle camera path. This fallback
	// keeps nonzero camera modes functional until that path is audited.
	(void)cDC;
	CAM_LookAtPosition(scratchpad, &d->posCurr.x, pushBufferPos, pushBufferRot);
}
