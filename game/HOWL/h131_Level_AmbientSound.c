#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002ebe4-0x8002f0dc
void Level_AmbientSound(void)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Level *level = gGT->level1;
	u32 levelID = gGT->levelID;
	int closestDistance[2];
	int closestIndex[2];

	if ((levelID >= 0x19) || ((u8)gGT->numPlyrCurrGame >= 3))
		return;

	if (levelID == 6)
	{
		bool playDrops = false;
		bool playLoop = false;

		for (int i = 0; i < (u8)gGT->numPlyrCurrGame; i++)
		{
			struct Driver *driver = gGT->drivers[i];
			char terrain = driver->currentTerrain;
			s16 sound = driver->terrainMeta2->sound;

			if ((terrain == 0) || (terrain == 1) || (terrain == 11))
				playDrops = true;

			if ((sound != -1) && (sound == 0x87))
				playLoop = true;
		}

		if (playDrops)
			Level_RandomFX(&sdata->SoundFadeInput[0].unk, 0x86, 6, 0x5a, 0xff);

		Level_SoundLoopFade((int *)&sdata->SoundFadeInput[1], 0x87, playLoop ? 0xff : 0, 8);
		return;
	}

	if (levelID == 8)
	{
		bool playFirstLoop = false;
		bool playSecondLoop = false;

		for (int i = 0; i < (u8)gGT->numPlyrCurrGame; i++)
		{
			s16 sound = gGT->drivers[i]->terrainMeta2->sound;

			if (sound != -1)
			{
				if (sound == 0x88)
					playFirstLoop = true;

				if (sound == 0x8b)
					playSecondLoop = true;
			}
		}

		Level_SoundLoopFade((int *)&sdata->SoundFadeInput[0], 0x88, playFirstLoop ? 0xff : 0, 8);
		Level_SoundLoopFade((int *)&sdata->SoundFadeInput[1], 0x8b, playSecondLoop ? 0xff : 0, 4);
		return;
	}

	for (int i = 0; i < 2; i++)
	{
		closestDistance[i] = 0x7fffffff;
		closestIndex[i] = -1;
	}

	for (int soundSlot = 0; soundSlot < 2; soundSlot++)
	{
		u32 soundID = data.levAmbientSound[levelID * 2 + soundSlot];
		int spawnIndex = soundSlot + 5;

		if (soundID == 0)
			continue;

		if (spawnIndex < level->numSpawnType2)
		{
			struct SpawnType2 *spawn = &level->ptrSpawnType2[spawnIndex];

			if (spawn->numCoords > 9)
				goto invalidSpawn;

			for (int coordIndex = 0; coordIndex < spawn->numCoords; coordIndex++)
			{
				s16 *coord = &spawn->posCoords[coordIndex * 3];

				for (int playerIndex = 0; playerIndex < (u8)gGT->numPlyrCurrGame; playerIndex++)
				{
					int distance = GTE_GetSquaredDistance(gGT->pushBuffer[playerIndex].pos, coord);

					if (distance < closestDistance[soundSlot])
					{
						closestDistance[soundSlot] = distance;
						closestIndex[soundSlot] = coordIndex;
					}
				}
			}

			{
				int distance = SquareRoot0_stub(closestDistance[soundSlot]);

				if (soundSlot == 0)
				{
					if (levelID == 9)
					{
						int volume = VehCalc_MapToRange(distance, 300, 6000, 0xff, 0);
						Level_RandomFX(&sdata->SoundFadeInput[0].unk, 0x86, 6, 0x5a, volume);
					}
					else
					{
						CalculateVolumeFromDistance((u32 *)&sdata->SoundFadeInput[0].soundID_soundCount, soundID, distance);
					}
				}
				else if (levelID == 3)
				{
					int volume = VehCalc_MapToRange(distance, 300, 6000, 0xff, 0);
					Level_RandomFX(&sdata->SoundFadeInput[1].unk, 0x85, 6, 0x5a, volume);
				}
				else
				{
					CalculateVolumeFromDistance((u32 *)&sdata->SoundFadeInput[1].soundID_soundCount, soundID, distance);
				}
			}
		}
		else
		{
		invalidSpawn:
			if (sdata->audioDefaults[6] == 0)
				sdata->audioDefaults[6] = 1;
		}
	}
}
