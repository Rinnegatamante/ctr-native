#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80067a74-0x80067b7c.
void VehStuckProc_RevEngine_PhysLinear(struct Thread *t, struct Driver *d)
{
	u32 unkTimer;

	struct GameTracker *gGT = sdata->gGT;

	unkTimer = (u16)d->KartStates.RevEngine.unk58e;
	unkTimer -= (u16)gGT->elapsedTimeMS;
	if ((unkTimer & 0x8000) != 0)
		unkTimer = 0;
	d->KartStates.RevEngine.unk58e = (s16)unkTimer;

	unkTimer = (u16)d->KartStates.RevEngine.unk590;
	unkTimer -= (u16)gGT->elapsedTimeMS;
	if ((unkTimer & 0x8000) != 0)
		unkTimer = 0;
	d->KartStates.RevEngine.unk590 = (s16)unkTimer;

	DECOMP_VehPhysProc_Driving_PhysLinear(t, d);

	if (d->KartStates.RevEngine.boolMaskGrab == 0)
		return;

	d->posCurr.y -= 0x200;

	// if maskObj exists
	if (d->KartStates.RevEngine.maskObj != 0)
		d->KartStates.RevEngine.maskObj->duration = 7680;

	struct CameraDC *cDC = &gGT->cameraDC[d->driverID];
	cDC->flags |= 0x10;
	cDC->unk98 = 0x40;
}
