#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80039fa8-0x8003a2b4
void MainGameEnd_SoloRaceGetReward(int subtractTimeCrateBonus)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *driver = gGT->drivers[0];
	struct Driver *player = gGT->threadBuckets[PLAYER].thread->object;

	gGT->newHighScoreIndex = -1;
	gGT->gameModeEnd &= 0x7bffffff;

	int timeBonus = 0;

	if (((s8)driver->numTimeCrates == gGT->timeCratesInLEV) && (subtractTimeCrateBonus != 0))
	{
		timeBonus = 0x2580;
	}

	int raceTime = driver->timeElapsedInRace - timeBonus;

	for (s16 i = 0; i < 5; i++)
	{
		if (raceTime < (s32)sdata->ptrActiveHighScoreEntry[i + 1].time)
		{
			gGT->newHighScoreIndex = i;
			gGT->gameModeEnd |= 0x88000000;
			break;
		}
	}

	gGT->bestLapTime = sdata->ptrActiveHighScoreEntry[0].time;

	for (s16 i = 0; i < (s8)gGT->numLaps; i++)
	{
		if (gGT->lapTime[i] < gGT->bestLapTime)
		{
			gGT->bestLapTime = gGT->lapTime[i];
			gGT->lapIndexNewBest = i;
			gGT->gameModeEnd |= 0x8c000000;
		}
	}

	if ((gGT->gameMode1 & TIME_TRIAL) == 0)
		return;

	if ((gGT->gameModeEnd & 4) != 0)
		return;

	gGT->gameModeEnd |= 4;

	struct HighScoreTrack *track = &sdata->gameProgress.highScoreTracks[gGT->levelID];
	int playerTime = player->timeElapsedInRace;

	if (((track->timeTrialFlags >> data.bitIndex_timeTrialFlags_saveData.nTropyOpen) & 1) == 0)
	{
		if ((track->timeTrialFlags & 1) == 0)
		{
			if (data.metaDataLEV[gGT->levelID].timeTrial <= playerTime)
				goto CheckOxideAllTracks;

			track->timeTrialFlags |= 1;
			gGT->gameModeEnd |= 0x08008000;
		}
		else
		{
			if (gGT->timeToBeatInTimeTrial_ForCurrentEvent <= playerTime)
				goto CheckOxideAllTracks;

			track->timeTrialFlags |= 1 << (data.bitIndex_timeTrialFlags_saveData.nTropyOpen & 0x1f);
			gGT->gameModeEnd |= data.bitIndex_timeTrialFlags_flashingText.nOxideOpen;
		}
	}
	else
	{
		if (gGT->timeToBeatInTimeTrial_ForCurrentEvent <= playerTime)
			goto CheckOxideAllTracks;

		track->timeTrialFlags |= 1 << (data.bitIndex_timeTrialFlags_saveData.nOxideOpen & 0x1f);
		gGT->gameModeEnd |= data.bitIndex_timeTrialFlags_flashingText.nTropyOpen;
	}

CheckOxideAllTracks:
	if ((GAMEPROG_CheckGhostsBeaten(2) & 0xffff) != 0)
	{
		sdata->gameProgress.unlocks[1] |= 0x10;
	}
}
