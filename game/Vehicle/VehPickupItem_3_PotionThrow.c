#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800652c8-0x8006540c.
u32 VehPickupItem_PotionThrow(struct MineWeapon *mine, struct Instance *inst, u32 flags)
{
	s32 throwVelocity;

	if ((flags & 4) == 0)
	{
		if ((flags & 2) == 0)
		{
			if ((flags & 1) == 0)
				return 0;

			throwVelocity = (MixRNG_Scramble() & 0x1f) - 0x10;
		}
		else
		{
			throwVelocity = -0x78;
		}
	}
	else
	{
		throwVelocity = 0x78;
	}

	mine->velocity[0] = (inst->matrix.m[0][2] * throwVelocity) >> 12;
	mine->velocity[1] = 0x30;
	mine->velocity[2] = (inst->matrix.m[2][2] * throwVelocity) >> 12;
	mine->crateInst = NULL;
	mine->extraFlags |= 2;

	return 1;
}
