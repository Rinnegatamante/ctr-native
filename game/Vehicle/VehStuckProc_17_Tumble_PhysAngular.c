#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80068150-0x80068244
void VehStuckProc_Tumble_PhysAngular(struct Thread *thread, struct Driver *driver)
{
	int elapsedTimeMS = sdata->gGT->elapsedTimeMS;

	driver->numFramesSpentSteering = 10000;

	driver->unk3D4[0] -= (driver->unk3D4[0] >> 3);
	driver->rotationSpinRate -= (driver->rotationSpinRate >> 3);
	driver->unk_LerpToForwards -= (driver->unk_LerpToForwards >> 3);

	driver->ampTurnState = driver->rotationSpinRate;

	driver->turnAngleCurr += driver->unk_LerpToForwards;
	driver->turnAngleCurr += 0x800;
	driver->turnAngleCurr &= 0xfff;
	driver->turnAngleCurr -= 0x800;

	driver->angle += (s16)(((int)driver->rotationSpinRate * elapsedTimeMS) >> 0xd);
	driver->angle &= 0xfff;

	(driver->rotCurr).y = driver->unk3D4[0] + driver->angle + driver->turnAngleCurr;

	(driver->rotCurr).w = VehCalc_InterpBySpeed((int)(driver->rotCurr).w, (elapsedTimeMS << 5) >> 5, 0);

	VehPhysForce_RotAxisAngle(&driver->matrixMovingDir, (s16 *)&driver->AxisAngle1_normalVec, driver->angle);
}
