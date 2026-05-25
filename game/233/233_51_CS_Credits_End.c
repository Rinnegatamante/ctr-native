#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b93f4-0x800b9488
void CS_Credits_End(void)
{
	int levID;
	struct GameTracker *gGT = sdata->gGT;

	// erase 5 instances
	CS_Credits_DestroyCreditGhost();

	// kill thread
	creditsBSS.CreditThread->flags |= 0x800;

	// go to gemstone valley
	if (creditsBSS.boolAllBlue == 0)
	{
		levID = GEM_STONE_VALLEY;

		gGT->gameMode1 |= ADVENTURE_MODE;
	}

	// go to scrapbook
	else
	{
		sdata->mainMenuState = 5;

		levID = SCRAPBOOK;
	}

	MainRaceTrack_RequestLoad(levID);

	gGT->renderFlags &= 0xfffffffb;
}
