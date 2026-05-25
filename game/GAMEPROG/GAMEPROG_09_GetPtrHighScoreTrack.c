#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80026e80-0x80026ed8
void GAMEPROG_GetPtrHighScoreTrack(void)
{
	int gameMode1;
	struct GameTracker *gGT;

	gGT = sdata->gGT;
	gameMode1 = gGT->gameMode1;

	sdata->ptrActiveHighScoreEntry = &sdata->gameProgress.highScoreTracks[gGT->levelID].scoreEntry[6 * ((gameMode1 & RELIC_RACE) != 0)];
}
