#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80064320-0x800643d4
void VehPhysProc_SpinStop_PhysAngular(struct Thread *t, struct Driver *d)
{
	int elapsedTimeMS = sdata->gGT->elapsedTimeMS;

	d->angle = (d->angle + ((d->ampTurnState * elapsedTimeMS) >> 0xd)) & 0xfff;
	d->rotCurr.y = d->unk3D4[0] + d->angle + d->turnAngleCurr;

	d->rotCurr.w = VehCalc_InterpBySpeed(d->rotCurr.w, elapsedTimeMS, 0);
	d->turnAngleCurr = VehCalc_InterpBySpeed(d->turnAngleCurr, elapsedTimeMS << 2, 0);

	VehPhysForce_RotAxisAngle(&d->matrixMovingDir, &d->AxisAngle1_normalVec.x, d->angle);
}

void DECOMP_VehPhysProc_SpinStop_PhysAngular(struct Thread *t, struct Driver *d)
{
	VehPhysProc_SpinStop_PhysAngular(t, d);
}
