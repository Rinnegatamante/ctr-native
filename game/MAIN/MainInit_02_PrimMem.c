#include <common.h>

void MainInit_PrimMem(struct GameTracker *gGT)

{
	int GetOriginalSize(struct GameTracker * gGT);
	int size = GetOriginalSize(gGT);

	MainDB_PrimMem(&gGT->db[0].primMem, size);
	MainDB_PrimMem(&gGT->db[1].primMem, size);
}

int GetOriginalSize(struct GameTracker *gGT)
{
	int levelID = gGT->levelID;

	// adv garage
	if (levelID == ADVENTURE_GARAGE)
		return 0x1b800;

	// main menu
	if (levelID == MAIN_MENU_LEVEL)
		return 0x17c00;

	if (gGT->numPlyrCurrGame == 1)
	{
		// any% end, 101% end, credits
		if (levelID >= OXIDE_ENDING)
			return 0x17c00;

		// intro cutscene
		if (levelID >= INTRO_RACE_TODAY)
			return 0x1e000;

		// adv hub
		if (levelID >= GEM_STONE_VALLEY)
			return 0x1c000;

		// ordinary tracks

		// all are 0x67 or 0x5F, adv hub was 0x5F too
		return data.primMem_SizePerLEV_1P[levelID] << 10;
	}

	if (gGT->numPlyrCurrGame == 2)
	{
		// assume only levID 0-24
		return data.primMem_SizePerLEV_2P[levelID] << 10;
	}

	// 3P 4P
	// assume only levID 0-24
	return data.primMem_SizePerLEV_4P[levelID] << 10;
}
