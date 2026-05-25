#include <common.h>

void CAM_StartLine_FlyIn(struct FlyInData *flyInData, s16 maxFrames, int frame, s16 *desiredPos, s16 *desiredRot)
{
	struct Level *lev = sdata->gGT->level1;
	int frameIndex = (frame << 0x10) >> 4;
	int frameRatio = frameIndex / maxFrames;
	int countEnd = flyInData->frameCount1;
	s16 count = flyInData->frameCount2;
	SVECTOR local_78;
	SVECTOR local_70;
	SVECTOR rot;
	MATRIX matrix;
	VECTOR transformed;
	s32 flags[2];
	s16 *pathEnd;
	s16 *pathStart;
	int pathRatioEnd;

	if (count < countEnd)
		count = countEnd;

	s16 pathIndex = (s16)(count * frameRatio >> 0xc);
	if (pathIndex < countEnd - 1)
	{
		pathEnd = (s16 *)(flyInData->ptrEnd + pathIndex * 6);
		pathRatioEnd = frameRatio;
	}
	else
	{
		pathEnd = (s16 *)(flyInData->ptrEnd + countEnd * 6 - 0xc);
		pathRatioEnd = 0;
	}

	if (pathIndex < flyInData->frameCount2 - 1)
	{
		pathStart = (s16 *)(flyInData->ptrStart + pathIndex * 6);
	}
	else
	{
		pathStart = (s16 *)(flyInData->ptrStart + flyInData->frameCount2 * 6 - 0xc);
		frameRatio = 0;
	}

	int ratio = count * pathRatioEnd & 0xfff;
	local_78.vx = pathEnd[0] + (s16)(((pathEnd[3] - pathEnd[0]) * ratio) >> 0xc);
	local_78.vy = pathEnd[1] + (s16)(((pathEnd[4] - pathEnd[1]) * ratio) >> 0xc);
	local_78.vz = pathEnd[2] + (s16)(((pathEnd[5] - pathEnd[2]) * ratio) >> 0xc);

	ratio = count * frameRatio & 0xfff;
	local_70.vx = pathStart[0] + (s16)(((pathStart[3] - pathStart[0]) * ratio) >> 0xc);
	local_70.vy = pathStart[1] + (s16)(((pathStart[4] - pathStart[1]) * ratio) >> 0xc) - 0x60;
	local_70.vz = pathStart[2] + (s16)(((pathStart[5] - pathStart[2]) * ratio) >> 0xc);

	rot.vx = lev->DriverSpawn[0].rot[0];
	rot.vy = lev->DriverSpawn[0].rot[1] + 0x400;
	rot.vz = lev->DriverSpawn[0].rot[2];

	ConvertRotToMatrix(&matrix, (s16 *)&rot);

	CAM_StartLine_FlyIn_FixY(&lev->DriverSpawn[1].pos[0]);
	CAM_StartLine_FlyIn_FixY(&lev->DriverSpawn[2].pos[0]);
	CAM_StartLine_FlyIn_FixY(&lev->DriverSpawn[5].pos[0]);

	matrix.t[0] = lev->DriverSpawn[1].pos[0] + (lev->DriverSpawn[2].pos[0] - lev->DriverSpawn[1].pos[0]) / 2;
	matrix.t[1] = lev->DriverSpawn[1].pos[1] + (lev->DriverSpawn[2].pos[1] - lev->DriverSpawn[1].pos[1]) / 2 + 0x40;
	matrix.t[2] = lev->DriverSpawn[1].pos[2] + (lev->DriverSpawn[2].pos[2] - lev->DriverSpawn[1].pos[2]) / 2;

	SetRotMatrix(&matrix);
	SetTransMatrix(&matrix);

	RotTrans(&local_78, &transformed, (long *)flags);
	desiredPos[0] = (s16)transformed.vx;
	desiredPos[1] = (s16)transformed.vy;
	desiredPos[2] = (s16)transformed.vz;

	RotTrans(&local_70, &transformed, (long *)flags);

	s16 deltaX = desiredPos[0] - (s16)transformed.vx;
	s16 deltaY = desiredPos[1] - (s16)transformed.vy;
	s16 deltaZ = desiredPos[2] - (s16)transformed.vz;

	desiredRot[1] = (s16)ratan2(deltaX, deltaZ);
	desiredRot[0] = 0x800 - (s16)ratan2(deltaY, SquareRoot0(deltaX * deltaX + deltaZ * deltaZ));
	desiredRot[2] = 0;
}
