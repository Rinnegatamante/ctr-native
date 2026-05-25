#include <common.h>

void MainFreeze_MenuPtrQuit(struct RectMenu *menu)
{
	s16 row;
	struct GameTracker *gGT = sdata->gGT;

	if (menu->unk1e == 0)
	{
		row = menu->rowSelected;
		if (row == 0)
		{
			// Erase ghost of previous race from RAM
			GhostTape_Destroy();

			// Add bit for "in menu" when loading is done
			sdata->Loading.OnBegin.AddBitsConfig0 |= MAIN_MENU;

			// Go to main menu
			sdata->mainMenuState = 0;

			// Remove bit for "In Adventure Arena" when loading is done
			sdata->Loading.OnBegin.RemBitsConfig0 |= ADVENTURE_ARENA;

			// Unpause game
			gGT->gameMode1 &= ~1;

			// Level ID for main menu (39)
			MainRaceTrack_RequestLoad(0x27);
			return;
		}

		// row == 1

		sdata->ptrActiveMenu = MainFreeze_GetMenuPtr();
	}
	else
	{
		menu->drawStyle &= ~0x100;

		// if more than 2 screens
		if (gGT->numPlyrCurrGame > 2)
		{
			menu->drawStyle |= 0x100;
		}

		MainFreeze_SafeAdvDestroy();
	}
	return;
}
