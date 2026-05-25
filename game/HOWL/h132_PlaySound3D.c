#include <common.h>

static u32 PlaySound3D_CalculateLR(s32 *dir)
{
	int lr = (ratan2(dir[0], -dir[2]) + 0x800) * -0x100000 >> 0x17;

	if (lr >= 0x81)
		lr = 0x100 - lr;
	else if (lr <= -0x81)
		lr = -0x100 - lr;

	lr += 0x80;
	if (lr < 0)
		return 0;

	if (lr > 0xff)
		return 0xff;

	return lr;
}

static u32 PlaySound3D_BuildFlags(struct GameTracker *gGT, int cameraIndex, u32 distance, u32 lr)
{
	u32 volume;
	u32 echo = ((u32)gGT->cameraDC[cameraIndex].ptrQuadBlock->quadFlags & 0x80) << 0x18;

	if (distance < 301)
		volume = 0xff;
	else
		volume = VehCalc_MapToRange(distance, 300, 9000, 0xff, 0);

	return echo | ((volume & 0xff) << 0x10) | (lr & 0xff) | 0x8000;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002f0dc-0x8002f31c
void PlaySound3D(u32 soundID, struct Instance *inst)
{
	struct GameTracker *gGT = sdata->gGT;
	s32 dir[4][3];
	u32 distance[4];
	u32 closestDistance = 9000;
	int closestCamera = 0;

	for (int i = 0; i < (u8)gGT->numPlyrCurrGame; i++)
	{
		dir[i][0] = inst->matrix.t[0] - gGT->pushBuffer[i].pos[0];
		dir[i][1] = inst->matrix.t[1] - gGT->pushBuffer[i].pos[1];
		dir[i][2] = inst->matrix.t[2] - gGT->pushBuffer[i].pos[2];

		distance[i] = GTE_GetSquaredLength(dir[i]);
		distance[i] = SquareRoot0_stub(distance[i]);

		if (distance[i] < closestDistance)
		{
			closestCamera = i;
			closestDistance = distance[i];
		}
	}

	if (closestDistance == 9000)
		return;

	GTE_AudioLR_Inst(&gGT->pushBuffer[closestCamera].matrix_Camera, dir[closestCamera]);

	DECOMP_OtherFX_Play_LowLevel(soundID & 0xffff, 1, PlaySound3D_BuildFlags(gGT, closestCamera, closestDistance, PlaySound3D_CalculateLR(dir[closestCamera])));
}
