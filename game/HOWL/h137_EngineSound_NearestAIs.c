#include <common.h>

static int EngineSound_NearestAIs_GetDistance(struct Driver *ai, int pushBufferIndex)
{
	struct PushBuffer *pb = &sdata->gGT->pushBuffer[pushBufferIndex];
	int dx = pb->pos[0] - (ai->posCurr.x >> 8);
	int dy = pb->pos[1] - (ai->posCurr.y >> 8);
	int dz = pb->pos[2] - (ai->posCurr.z >> 8);

	return SquareRoot0_stub(dx * dx + dy * dy + dz * dz);
}

static void EngineSound_NearestAIs_InsertClosest(struct Driver *ai, int playerIndex, int distance, struct Driver **closestDrivers, int *closestDistances,
                                                 s16 *closestPlayers)
{
	if (distance < closestDistances[0])
	{
		closestPlayers[1] = closestPlayers[0];
		closestDrivers[1] = closestDrivers[0];
		closestDistances[1] = closestDistances[0];

		closestPlayers[0] = playerIndex;
		closestDrivers[0] = ai;
		closestDistances[0] = distance;
	}
	else if (distance < closestDistances[1])
	{
		closestPlayers[1] = playerIndex;
		closestDrivers[1] = ai;
		closestDistances[1] = distance;
	}
}

static int EngineSound_NearestAIs_CalculateLR(s32 *dir)
{
	int lr = (ratan2(dir[0], -dir[2]) + 0x800) * -0x100000 >> 0x17;

	if (lr < 0x81)
	{
		if (lr < -0x80)
			lr = -0x100 - lr;
	}
	else
	{
		lr = 0x100 - lr;
	}

	return lr + 0x80;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002ff28-0x80030208
void EngineSound_NearestAIs(void)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *closestDrivers[2];
	int closestDistances[2];
	s16 closestPlayers[2];

	if (gGT->numBotsCurrGame == 0)
		return;

	closestDrivers[0] = NULL;
	closestDrivers[1] = NULL;
	closestDistances[0] = 0x7fffffff;
	closestDistances[1] = 0x7fffffff;

	for (struct Thread *thread = gGT->threadBuckets[ROBOT].thread; thread != NULL; thread = thread->siblingThread)
	{
		struct Driver *ai = thread->object;

		for (int i = 0; i < (u8)gGT->numPlyrCurrGame; i++)
			EngineSound_NearestAIs_InsertClosest(ai, i, EngineSound_NearestAIs_GetDistance(ai, i), closestDrivers, closestDistances, closestPlayers);
	}

	for (int i = 0; i < 2; i++)
	{
		struct Driver *ai = closestDrivers[i];
		if (ai != NULL)
		{
			s32 dir[3];
			s16 playerIndex = closestPlayers[i];
			struct Driver *cameraDriver = gGT->cameraDC[playerIndex].driverToFollow;
			u32 lr;

			GTE_AudioLR_Driver(&gGT->pushBuffer[playerIndex].matrix_Camera, ai, dir);

			lr = EngineSound_VolumeAdjust(EngineSound_NearestAIs_CalculateLR(dir), sdata->audioDefaults[4 + i], 10);
			sdata->audioDefaults[4 + i] = lr;

			EngineSound_AI(ai, cameraDriver, i, closestDistances[i], closestDistances[i] - sdata->audioDefaults[2 + i], lr);
			sdata->audioDefaults[2 + i] = closestDistances[i];
		}
	}
}
