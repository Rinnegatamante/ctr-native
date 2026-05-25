#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002de48-0x8002e338
u32 Music_AsyncParseBanks(void)
{
	u8 bVar1;
	u16 index;
	u32 uVar4;
	struct GameTracker *gGT = sdata->gGT;
	struct Bank thisBank;
	int level = gGT->levelID;
	u8 *arr = (u8 *)&sdata->audioDefaults[7];

	if (arr[1] == 5)
		return 1;
	if (arr[1] > 4)
		return 0;

	// loading state of song (one byte)
	switch (arr[1])
	{
	// Load Level FX Bank
	case 0:
		// unsuccessful
		if (Bank_AssignSpuAddrs() == 0)
			goto PARSE_FINISH;

		// If you're in a Boss Race
		if (gGT->gameMode1 < 0)
		{
			if (gGT->bossID >= 6)
				break;

			index = sdata->songBankBossID[gGT->bossID];
		}

		// any driving track
		else if (level < INTRO_RACE_TODAY)
		{
			index = data.levBank_FX[level];
		}

		else if (level == MAIN_MENU_LEVEL)
		{
			index = 0x20;
		}

		// If you're drawing Any% ending cutscene
		else if (level == OXIDE_ENDING)
		{
			index = 0x23;
		}

		// If you're drawing 100% ending cutscene
		else if (level == OXIDE_TRUE_ENDING)
		{
			index = 0x24;
		}

		// if you're drawing credits
		else if (level == CREDITS_CRASH)
		{
			index = 0x25;
		}

		// go to next stage
		else
			break;

		Bank_Load(index, &thisBank);

		// go to next stage
		break;

	// Finish Song Bank, Start Default8 Bank
	case 1:
		// unsuccessful
		if (Bank_AssignSpuAddrs() == 0)
			goto PARSE_FINISH;

		sdata->bankCount = 0;
		sdata->bankPodiumStage = 0;
		sdata->bankLoad54 = 0;

		// if not driving track, skip
		if (level >= GEM_STONE_VALLEY)
			break;

		uVar4 = gGT->gameMode1;

		if (

		    // If this is a race with 8 drivers

		    (
		        // If you're in Adventure Mode
		        (uVar4 & ADVENTURE_MODE) != 0 &&

		        // Not in any of these...
		        // Boss Race or Adventure Arena
		        // Or Crystal Challenge or Relic Race
		        ((uVar4 & 0x8c100000) == 0) &&

		        // If not purple gem cup
		        (((uVar4 & ADVENTURE_CUP) == 0) || (gGT->cup.cupID != 4))

		            ) ||

		    ((uVar4 & ARCADE_MODE) != 0))
		{
			// Load bank with 8 drivers
			Bank_Load(54, &thisBank);
			sdata->bankLoad54 = 1;
		}

		// go to next stage
		break;

	// Finish Default8 Bank, start Character Bank
	case 2:

		if (Bank_AssignSpuAddrs() == 0)
			goto PARSE_FINISH;

		// If you're on any Arcade or Battle map
		if (level < GEM_STONE_VALLEY)
		{
			// If you're not in Adventure Cup
			if (((gGT->gameMode1 & ADVENTURE_MODE) == 0) ||

			    // If you're not in Adventure Cup
			    ((gGT->gameMode1 & ADVENTURE_CUP) == 0) ||

			    // If this is not the purple gem cup
			    (gGT->cup.cupID != 4))
			{
				// numPlyrCurrGame
				if (sdata->bankCount < gGT->numPlyrCurrGame)
				{
					if (
					    // if bank of 8 drivers is loaded
					    (sdata->bankLoad54 != 0) &&

					    // if characterID is part of original 8
					    (data.characterIDs[sdata->bankCount] < PINSTRIPE))
					{
						// skip load bank
						goto LAB_8002e178;
					}

					// load bank for character
					index = data.characterIDs[sdata->bankCount] + 0x37;
					goto LOAD_BANK;
				}
			}

			// if purple gem cup
			else
			{
				// load 5 banks, one for each driver
				if (sdata->bankCount < 5)
				{
					index = data.characterIDs[sdata->bankCount] + 0x37;

				LOAD_BANK:

					Bank_Load(index, &thisBank);

				LAB_8002e178:
					sdata->bankCount = sdata->bankCount + 1;
					goto PARSE_FINISH;
				}
			}
		}

		// if adventure arena
		else if (level < INTRO_RACE_TODAY)
		{
			if (sdata->bankCount == 0)
			{
				if ((sdata->bankLoad54 != 0) ||

				    // characterID is special character
				    (PURA < data.characterIDs[0]))
				{
					goto LAB_8002e178;
				}

				// bank = characterID + 0x37
				index = data.characterIDs[0] + 0x37;

				// load bank
				goto LOAD_BANK;
			}

			// if no podium reward, or all podium banks
			// are loaded, then go to next "case 3:" stage
			if (gGT->podiumRewardID == NOFUNC)
				break; // 0
			if (sdata->bankPodiumStage >= 3)
				break;

			switch (sdata->bankPodiumStage)
			{
			case 0:
				bVar1 = gGT->podium_modelIndex_First;
				break;

			case 1:
				if (gGT->podium_modelIndex_Second == 0)
					goto PODIUM_STAGE_DONE;

				bVar1 = gGT->podium_modelIndex_Second;
				break;

			default:
				if (gGT->podium_modelIndex_Third == 0)
					goto PODIUM_STAGE_DONE;

				bVar1 = gGT->podium_modelIndex_Third;
				break;
			}

			Bank_Load((u16)(bVar1 - 88), &thisBank);

		PODIUM_STAGE_DONE:
			sdata->bankPodiumStage++;
			goto PARSE_FINISH;
		}
		break;

	// Finish Character Bank, Start Level Song Bank
	case 3:

		if (Bank_AssignSpuAddrs() == 0)
			goto PARSE_FINISH;

		// If you're in a Boss Race
		// 0x80000000
		if (gGT->gameMode1 < 0)
		{
			if (gGT->bossID >= 6)
				break;

			index = 0x19;
		}

		// any level you can drive on
		else if (level < INTRO_RACE_TODAY)
		{
			// Set Song ID depending on track
			index = data.levBank_Song[level];
		}

		else if (level == MAIN_MENU_LEVEL)
		{
			index = 27;
		}

		else if (level == INTRO_RACE_TODAY)
		{
			index = 29;
		}

		else if (level == NAUGHTY_DOG_CRATE)
		{
			index = 28;
		}

		// any%
		else if (level == OXIDE_ENDING)
		{
			index = 30;
		}

		// 100% ending
		else if (level == OXIDE_TRUE_ENDING)
		{
			index = 31;
		}

		else if (level == CREDITS_CRASH)
		{
			index = 32;
		}

		else
			break;

		DECOMP_howl_SetSong(index);
		break;

	// Finish Level Song Bank
	case 4:

		if (DECOMP_howl_LoadSong() == 0)
			goto PARSE_FINISH;

		// go to next stage
		break;

	default:
		return 0;
	}

	// loading state of song (one byte)
	arr[1]++;

PARSE_FINISH:
	// state == 5 is finished.
	// if false, function failed.
	return (arr[1] == 5);
}
