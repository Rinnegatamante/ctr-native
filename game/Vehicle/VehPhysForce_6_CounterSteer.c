#include <common.h>

static SVec3 VehPhysForce_CounterSteer_RotateVector(const MATRIX *m, s16 vx, s16 vy, s16 vz)
{
	SVec3 out;

	out.x = (s16)(((int)m->m[0][0] * vx + (int)m->m[0][1] * vy + (int)m->m[0][2] * vz) >> 12);
	out.y = (s16)(((int)m->m[1][0] * vx + (int)m->m[1][1] * vy + (int)m->m[1][2] * vz) >> 12);
	out.z = (s16)(((int)m->m[2][0] * vx + (int)m->m[2][1] * vy + (int)m->m[2][2] * vz) >> 12);

	return out;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005fb4c-0x8005fc8c
void VehPhysForce_CounterSteer(struct Driver *driver)
{
	driver->accel.x = 0;
	driver->accel.y = 0;
	driver->accel.z = 0;

	int speedApprox = abs(driver->speedApprox);
	if (speedApprox <= FP8(3) || driver->actionsFlagSet & ACTION_WARP || driver->kartState == KS_CRASHING || driver->set_0xF0_OnWallRub ||
	    !(driver->actionsFlagSet & ACTION_TOUCH_GROUND) || driver->terrainMeta1->counterSteerRatio == 0)
	{
		return;
	}

	int angleLimit = (u8)driver->angleMaxCounterSteer;
	int angle = driver->turnAngleCurr - driver->turnAnglePrev;
	if (angle > angleLimit)
	{
		angle = angleLimit;
	}
	else if (angle < -angleLimit)
	{
		angle = -angleLimit;
	}

	int sine = MATH_Sin(angle);
	int counterSteerStrength = (driver->terrainMeta1->counterSteerRatio * -8000) >> 8;
	SVec3 accel = VehPhysForce_CounterSteer_RotateVector(&driver->matrixMovingDir, (s16)((counterSteerStrength * sine) >> 12), 0, 0);

	driver->accel = accel;
}
