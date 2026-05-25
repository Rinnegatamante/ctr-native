#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005cd1c-0x8005cf64
void VehPhysCrash_ConvertVecToSpeed(struct Driver *d, Vec3 *vel)
{
	int speed2D = VehCalc_FastSqrt((u32)(vel->x * vel->x + vel->z * vel->z), 0x10);
	s16 speed3D = (s16)(VehCalc_FastSqrt((u32)(vel->x * vel->x + vel->y * vel->y + vel->z * vel->z), 0x10) >> 8);

	d->speed = speed3D;
	d->axisRotationY = (s16)ratan2(vel->y << 8, speed2D);
	d->axisRotationX = (s16)ratan2(vel->x, vel->z);

	int projOnMovingDirAxis = (vel->x * d->matrixMovingDir.m[0][1] + vel->y * d->matrixMovingDir.m[1][1] + vel->z * d->matrixMovingDir.m[2][1]) >> 0xc;

	int projX = (d->matrixMovingDir.m[0][1] * projOnMovingDirAxis) >> 0xc;
	int projY = (d->matrixMovingDir.m[1][1] * projOnMovingDirAxis) >> 0xc;
	int projZ = (d->matrixMovingDir.m[2][1] * projOnMovingDirAxis) >> 0xc;

	speed3D = (s16)(VehCalc_FastSqrt((u32)(projX * projX + projY * projY + projZ * projZ), 0x10) >> 8);

	d->jumpHeightCurr = speed3D;
	if (projOnMovingDirAxis < 0)
	{
		d->jumpHeightCurr = -speed3D;
	}

	projX = vel->x - projX;
	projY = vel->y - projY;
	projZ = vel->z - projZ;

	speed3D = (s16)(VehCalc_FastSqrt((u32)(projX * projX + projY * projY + projZ * projZ), 0x10) >> 8);

	d->speedApprox = speed3D;

	if ((projX * d->matrixMovingDir.m[0][2] + projY * d->matrixMovingDir.m[1][2] + projZ * d->matrixMovingDir.m[2][2]) < 0)
	{
		d->speedApprox = -speed3D;
	}
}
