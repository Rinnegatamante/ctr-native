#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003a2b4-0x8003a3fc
void MainGameEnd_SoloRaceSaveHighScore(void)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *player = gGT->drivers[0];

	MainGameEnd_SoloRaceGetReward(0);

	u32 gameModeEnd = gGT->gameModeEnd;

	if ((gameModeEnd & HIGH_SCORE_SAVED) != 0)
		return;

	gGT->gameModeEnd = gameModeEnd | HIGH_SCORE_SAVED;

	struct HighScoreEntry *entry = sdata->ptrActiveHighScoreEntry;

	if ((gameModeEnd & NEW_BEST_LAP) != 0)
	{
		entry[0].time = gGT->bestLapTime;
		entry[0].name[0] = 0;
		entry[0].characterID = data.characterIDs[player->driverID];
		memmove(entry[0].name, gGT->prevNameEntered, 0x11);
	}

	int highScoreIndex = (s8)gGT->newHighScoreIndex;

	if (highScoreIndex < 0)
		return;

	entry = &sdata->ptrActiveHighScoreEntry[highScoreIndex + 1];

	if (highScoreIndex < 4)
	{
		memmove(&entry[1], entry, (4 - highScoreIndex) * sizeof(struct HighScoreEntry));
	}

	entry->time = player->timeElapsedInRace;
	entry->name[0] = 0;
	entry->characterID = data.characterIDs[player->driverID];
	memmove(entry->name, gGT->prevNameEntered, 0x11);
}
