#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80026ae4-0x80026bf0
int GAMEPROG_CheckGhostsBeaten(int ghostID)
{
	struct GameTracker *gGT = sdata->gGT;
	int result = 1;
	s16 levelID = gGT->levelID;
	int flagWordIndex = (s16)ghostID >> 5;

	for (int i = 0; i < 18; i++)
	{
		gGT->levelID = i;
		GAMEPROG_GetPtrHighScoreTrack();

		if (result != 0)
		{
			u32 *timeTrialFlags = &sdata->gameProgress.highScoreTracks[gGT->levelID].timeTrialFlags;
			result = (timeTrialFlags[flagWordIndex] >> (ghostID & 0x1f)) & 1;
		}
	}

	gGT->levelID = levelID;
	GAMEPROG_GetPtrHighScoreTrack();

	return result;
}
