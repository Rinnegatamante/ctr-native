#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80062e04-0x80062e94.
void VehPhysProc_FreezeVShift_ReverseOneFrame(struct Thread *t, struct Driver *d)
{
	VehPhysGeneral_JumpAndFriction(t, d);

	int actionFlagSet = d->actionsFlagSet;

	// if player did not start jumping this frame
	if ((actionFlagSet & 0x400) == 0)
	{
		// if there are not two humans colliding
		if ((actionFlagSet & 0x10000000) == 0)
		{
			d->xSpeed = 0;
			d->ySpeed = 0;
			d->zSpeed = 0;

			d->speed = 0;
			d->speedApprox = 0;

			// set position to previous position
			d->posCurr.x = d->posPrev.x;
			d->posCurr.y = d->posPrev.y;
			d->posCurr.z = d->posPrev.z;
		}

		return;
	}

	VehPhysProc_Driving_Init(t, d);
}
