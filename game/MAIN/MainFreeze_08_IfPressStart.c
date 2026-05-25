#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80039e98-0x80039fa8.
void MainFreeze_IfPressStart(void)
{
	struct GameTracker *gGT = sdata->gGT;

	// Check conditions for pausing the game
	if ((RaceFlag_IsFullyOnScreen() == 0) &&

	    // if you are not drawing loading screen (after fully off screen)
	    ((gGT->renderFlags & 0x1000) == 0) &&

	    (sdata->AkuAkuHintState == 0) && (sdata->ptrActiveMenu == 0) && ((gGT->gameMode1 & (END_OF_RACE | PAUSE_ALL | GAME_CUTSCENE)) == 0) &&
	    (gGT->levelID != MAIN_MENU_LEVEL) && (gGT->boolDemoMode == 0) && ((u32)(gGT->levelID - OXIDE_ENDING) > 1) && (sdata->load_inProgress == 0) &&
	    ((gGT->gameMode2 & 4) == 0))
	{
		// pause the game
		gGT->gameMode1 |= PAUSE_1;

		// set row selected to the top row
		struct RectMenu *menu = MainFreeze_GetMenuPtr();
		menu->rowSelected = 0;

		DECOMP_RECTMENU_Show(menu);

		// pause audio
		MainFrame_TogglePauseAudio(1);

		DECOMP_OtherFX_Play(1, 1);

		ElimBG_Activate(gGT);
	}
}
