#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800605a0-0x80060630
int VehPhysGeneral_JumpGetVelY(s16 *normalVec, Vec3 *speedXYZ)
{
	int normalY = normalVec[1];
	int absNormalY = normalY;

	if (absNormalY < 0)
	{
		absNormalY = -absNormalY;
	}

	if (absNormalY < 0x15)
	{
		return 0;
	}

	int dot = (speedXYZ->x * normalVec[0]) + (speedXYZ->z * normalVec[2]);

	return dot / normalY;
}
