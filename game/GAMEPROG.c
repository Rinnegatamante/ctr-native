#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800265c0-0x8002689c.
void GAMEPROG_AdvPercent(struct AdvProgress *adv)
{
	int i;
	int percent;
	int oxidePercent;
	int allGoldOrPlatinumRelics;
	int numGems;
	int bitIndex;
	struct MetaDataLEV *mdLev;
	struct GameTracker *gGT;
	gGT = sdata->gGT;
	mdLev = &data.metaDataLEV[0];

	// start counter
	percent = 0;
	oxidePercent = 0;
	allGoldOrPlatinumRelics = 1;
	numGems = 0;

	// erase counters
	for (i = 0; i < 9; i++)
	{
		((int *)&gGT->currAdvProfile.numTrophies)[i] = 0;
	}

	// check all tracks generically
	for (i = 0; i < 18; i++)
	{
		// first bit of blue relic
		bitIndex = 0x16 + i;
		if (CHECK_ADV_BIT(adv->rewards, bitIndex) != 0)
		{
			gGT->currAdvProfile.numRelics++;
		}

		// check 16 trophies
		if (i < 0x10)
		{
			// first bit of trophy
			bitIndex = 6 + i;
			if (CHECK_ADV_BIT(adv->rewards, bitIndex) != 0)
			{
				gGT->currAdvProfile.numTrophies++;
			}

			// first bit of token
			bitIndex = 0x4c + i;
			if (CHECK_ADV_BIT(adv->rewards, bitIndex) != 0)
			{
				// increment number of tokens, based on
				// the tokenID of this level (red, green, blue, etc)
				((int *)&gGT->currAdvProfile.numCtrTokens.red)[mdLev[i].ctrTokenGroupID]++;

				// increment number of total tokens
				gGT->currAdvProfile.numCtrTokens.total++;
			}
		}

		// check 4 keys, and 4 purple tokens
		if (i < 4)
		{
			// first bit of key
			bitIndex = 0x5e + i;
			if (CHECK_ADV_BIT(adv->rewards, bitIndex) != 0)
			{
				gGT->currAdvProfile.numKeys++;
			}

			// first bit of purple tokens
			bitIndex = 0x6f + i;
			if (CHECK_ADV_BIT(adv->rewards, bitIndex) != 0)
			{
				gGT->currAdvProfile.numCtrTokens.purple++;
			}
		}

		// check 5 gems
		if (i < 5)
		{
			// first bit of gem
			bitIndex = 0x6a + i;
			if (CHECK_ADV_BIT(adv->rewards, bitIndex) != 0)
			{
				numGems++;
			}
		}

		// first bit is 2%, second bit upgrades the total Oxide bonus to 3%
		if (i < 2)
		{
			// first bit of beating oxide
			bitIndex = 0x73 + i;
			if (CHECK_ADV_BIT(adv->rewards, bitIndex) != 0)
			{
				oxidePercent = (i == 0) ? 2 : 3;
			}
		}
	}

	// check whether all tracks have gold or platinum relic
	for (i = 0; i < 18; i++)
	{
		// first bit of gold relic
		bitIndex = 0x28 + i;
		if ((allGoldOrPlatinumRelics != 0) && (CHECK_ADV_BIT(adv->rewards, bitIndex) != 0))
		{
			// check next relic
			continue;
		}

		// if relic is not unlocked,
		// then extra 1% is not earned
		allGoldOrPlatinumRelics = 0;
	}

	percent += gGT->currAdvProfile.numRelics * 2 + gGT->currAdvProfile.numTrophies * 2 + gGT->currAdvProfile.numKeys + gGT->currAdvProfile.numCtrTokens.total +
	           gGT->currAdvProfile.numCtrTokens.purple + numGems + oxidePercent + allGoldOrPlatinumRelics;

	gGT->currAdvProfile.completionPercent = percent;
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002689c-0x80026ae4.
void GAMEPROG_ResetHighScores(struct GameProgress *gameProg)
{
	int i;
	int j;
	int k;
	int characterID;
	struct HighScoreTrack *track;
	struct HighScoreEntry *entry;
	char *name;

	// for every track
	for (i = 0; i < 18; i++)
	{
		track = &gameProg->highScoreTracks[i];

#if 0
		// all but two tracks
		if(i > 1)
		{
			// temporary test
			track->timeTrialFlags = 7;
		}
#endif

		// for time trial and relic
		for (j = 0; j < 2; j++)
		{
			// for every entry
			for (k = 0; k < 6; k++)
			{
				characterID = i + j + k;
				characterID = characterID - PENTA_PENGUIN * (characterID / PENTA_PENGUIN);

				entry = &track->scoreEntry[j * 6 + k];
				entry->time = 0x8c640;
				entry->characterID = characterID;

				name = sdata->lngStrings[data.MetaDataCharacters[characterID].name_LNG_short];

				// can't do an int-copy,
				// strings in LNG are unaligned
				strcpy(entry->name, name);
			}
		}
	}
}


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


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80026bf0-0x80026c24.
void GAMEPROG_NewProfile_OutsideAdv(struct GameProgress *gameProg)
{
	// GameOptions is probably a struct "inside"
	// of GameProgress, still working on it

	// GameProgress and GameOptions
	memset(gameProg, 0, sizeof(struct GameProgress) + sizeof(struct GameOptions));

	GAMEPROG_ResetHighScores(gameProg);
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80026c24-0x80026cb8.
void GAMEPROG_InitFullMemcard(struct MemcardProfile *mcp)
{
	int i;

	// clear
	memset(mcp, 0, sizeof(struct MemcardProfile));

	// header
	mcp->header[0] = 0xffee; // version (-18)
	mcp->header[1] = sizeof(struct MemcardProfile);

	// GameProgress and GameOptions
	GAMEPROG_NewProfile_OutsideAdv(&mcp->gameProgress);

	// 4 profiles
	for (i = 0; i < 4; i++)
	{
		// no character selected
		mcp->advProgress[i].characterID = -1;

		// N Sane Beach
		mcp->advProgress[i].HubLevYouSavedOn = 0x1a;
	}
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80026cb8-0x80026cf4.
void GAMEPROG_NewProfile_InsideAdv(struct AdvProgress *adv)
{
	// clear
	memset(adv, 0x0, sizeof(struct AdvProgress));

	// no character selected
	adv->characterID = -1;

	// N Sane Beach
	adv->HubLevYouSavedOn = 0x1a;
}


// same as the one in GAMEPROG_AdvPercent
#define CHECK_PROG_BIT(rewards, bitIndex) ((rewards[bitIndex >> 5] >> (bitIndex & 0x1f)) & 1) != 0

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80026cf4-0x80026d7c.
void GAMEPROG_SaveCupProgress(void)
{
	int i;
	int *prog;
	int bitIndex1;
	int bitIndex2;

	prog = &sdata->gameProgress.unlocks[0];

	// 4 cups, 3 difficulties
	for (i = 0; i < 12; i++)
	{
		// if cup is "currently" beaten
		bitIndex1 = i + 0xC;
		if (CHECK_PROG_BIT(prog, bitIndex1))
		{
			// set if cup was "previously" beaten
			bitIndex2 = bitIndex1 + 0xC;
			prog[bitIndex2 >> 5] |= 1 << (bitIndex2 & 0x1f);
		}
	}
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80026d7c-0x80026e48.
void GAMEPROG_SyncGameAndCard(struct GameProgress *memcardProg, struct GameProgress *currentProg)
{
	int i;
	int memcardFlags;
	int currentFlags;
	int joinFlags;

	// combine progress of cups,
	// characters, track, scrapbook
	for (i = 0; i < 2; i++)
	{
		memcardFlags = memcardProg->unlocks[i];
		currentFlags = currentProg->unlocks[i];

		joinFlags = memcardFlags | currentFlags;

		memcardProg->unlocks[i] = joinFlags;
		currentProg->unlocks[i] = joinFlags;
	}

	// combine progress of beaten ghosts
	for (i = 0; i < 18; i++)
	{
		memcardFlags = memcardProg->highScoreTracks[i].timeTrialFlags;
		currentFlags = currentProg->highScoreTracks[i].timeTrialFlags;

		joinFlags = memcardFlags | currentFlags;

		memcardProg->highScoreTracks[i].timeTrialFlags = joinFlags;
		currentProg->highScoreTracks[i].timeTrialFlags = joinFlags;
	}

	// Naughty Dog left this incomplete
	// What if the game beat half of N Tropy's ghosts
	// and the new memory card beat the other half?
	// Now you have all ghosts beaten, but N Tropy
	// is still locked, and can't possibly be unlocked

	// Need to check cup flags for an unlocked battle track,
	// and n tropy ghosts for n tropy,
	// and oxide ghosts for scrapbook
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80026e48-0x80026e80.
void GAMEPROG_NewGame_OnBoot()
{
	GAMEPROG_NewProfile_OutsideAdv(&sdata->gameProgress);
	GAMEPROG_NewProfile_InsideAdv(&sdata->advProgress);
	GAMEPROG_GetPtrHighScoreTrack();
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80026e80-0x80026ed8
void GAMEPROG_GetPtrHighScoreTrack(void)
{
	int gameMode1;
	struct GameTracker *gGT;

	gGT = sdata->gGT;
	gameMode1 = gGT->gameMode1;

	sdata->ptrActiveHighScoreEntry = &sdata->gameProgress.highScoreTracks[gGT->levelID].scoreEntry[6 * ((gameMode1 & RELIC_RACE) != 0)];
}
