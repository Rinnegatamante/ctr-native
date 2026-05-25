#include <common.h>

static Vec3 VehPhysForce_OnApplyForces_RotateVector(const MATRIX *m, s16 vx, s16 vy, s16 vz)
{
	Vec3 out;

	out.x = ((int)m->m[0][0] * vx + (int)m->m[0][1] * vy + (int)m->m[0][2] * vz) >> 12;
	out.y = ((int)m->m[1][0] * vx + (int)m->m[1][1] * vy + (int)m->m[1][2] * vz) >> 12;
	out.z = ((int)m->m[2][0] * vx + (int)m->m[2][1] * vy + (int)m->m[2][2] * vz) >> 12;

	return out;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005ea60-0x8005ebac
void VehPhysForce_OnApplyForces(struct Thread *thread, struct Driver *driver)
{
	(void)thread;

	const int maxMudSinkYLevel = FP(-1);
	const int maxSpeed = FP8(100);
	driver->speed = min(driver->speed, maxSpeed);

	/* origin of driver model is center-bottom of kart,
	use orientation matrix, and half-radius {0, 25, 0},
	to find the "true" center of the 3D model */
	driver->originToCenter = VehPhysForce_OnApplyForces_RotateVector(&driver->matrixFacingDir, 0, 25, 0);

	VehPhysForce_ConvertSpeedToVecOut(driver, &driver->velocity);

	if ((driver->underDriver) && (driver->underDriver->terrain_type == TERRAIN_MUD))
	{
		if (driver->posCurr.y > maxMudSinkYLevel)
		{
			// sink slower as you approach the mud's bottom
			int sinkSpeed = maxMudSinkYLevel - driver->posCurr.y;
			driver->velocity.y = max(driver->velocity.y, sinkSpeed);
		}
	}

	VehPhysForce_OnGravity(driver, &driver->velocity);

	const SVec3 up = {.x = FP(0), .y = FP(1), .z = FP(0)};
	driver->normalVecUP = up;
	driver->AxisAngle1_normalVec = up;
	driver->unkAA = 0; // driver quadblock flags?
	driver->currBlockTouching = nullptr;

	driver->velocity.x += driver->accel.x;
	driver->velocity.z += driver->accel.z;
	driver->velocity.y += driver->accel.y;
}
