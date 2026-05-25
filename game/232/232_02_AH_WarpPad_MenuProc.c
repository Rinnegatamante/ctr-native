#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abd80-0x800abdfc.
void AH_WarpPad_MenuProc(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;

	RECTMENU_Hide(menu);

	if (menu->rowSelected == 0)
	{
		gGT->gameMode2 |= TOKEN_RACE;
	}

	else if (menu->rowSelected == 1)
	{
		gGT->gameMode1 |= RELIC_RACE;
	}
}
