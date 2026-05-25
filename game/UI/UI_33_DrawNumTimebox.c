#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80050e6c-0x80050f18.
void UI_DrawNumTimebox(s16 posX, s16 posY, struct Driver *d)
{
	int numCratesOwned;
	int numCratesTotal;
	struct GameTracker *gGT;
	char string[8];

	DecalFont_DrawLine(&sdata->s_x[0], posX + 0x14, posY - 10, FONT_SMALL, ORANGE);

	gGT = sdata->gGT;
	numCratesOwned = d->numTimeCrates;
	numCratesTotal = gGT->timeCratesInLEV;

	sprintf(&string[0], "%2.02d/%ld", numCratesOwned, numCratesTotal);

	DecalFont_DrawLine(&string[0], posX + 0x21, posY - 0xe, FONT_BIG, ORANGE);
}
