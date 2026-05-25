#include <common.h>

extern struct RectMenu menu224;

static s16 *SelectProfile_AllProfiles_Mode(void)
{
	return (s16 *)&sdata->data10_bbb[0];
}

static s16 *SelectProfile_AllProfiles_MemcardBusy(void)
{
	return (s16 *)&sdata->data10_bbb[2];
}

static s16 *SelectProfile_AllProfiles_ExitToPrevious(void)
{
	return (s16 *)&sdata->data10_bbb[4];
}

static s16 *SelectProfile_AllProfiles_OverwritePrompt(void)
{
	return (s16 *)&sdata->data10_bbb[8];
}

static int SelectProfile_GhostRowCount(void)
{
	int count = sdata->numGhostProfilesSaved;

	if (count < 0)
		count = 0;

	if (count > 7)
		count = 7;

#ifndef CTR_NATIVE
	if ((sdata->memcardAction == 1) && (sdata->memoryCard_SizeRemaining < 0x3e00))
		return count;
#endif

	if (count < 7)
		count++;

	return count;
}

static void SelectProfile_DrawGhostRows(struct RectMenu *menu, int rowCount)
{
	struct GameTracker *gGT = sdata->gGT;
	RECT box;
	char rowText[0x60];
	int i;
	int x;
	int y;
	int color;
	int pair;
	int rowHeight;

	box.x = 0x50;
	box.y = 0x32;
	box.w = 0x160;
	box.h = 0x84;
	DECOMP_RECTMENU_DrawInnerRect(&box, 4, gGT->backBuffer->otMem.startPlusFour);

	DECOMP_DecalFont_DrawLine("SAVE GHOST", 0x100, 0x3d, FONT_BIG, JUSTIFY_CENTER | ORANGE);

	rowHeight = (rowCount > 6) ? 0x2c : 0x30;

	for (i = 0; i < rowCount; i++)
	{
		pair = i >> 1;
		if ((i < rowCount - 1) || ((i & 1) != 0))
			x = ((i & 1) * 0xd4) + 0x2e;
		else
			x = 0x98;

		y = 0x58 + (pair * rowHeight);
		color = JUSTIFY_CENTER | WHITE;

		if (i == menu->rowSelected)
			color = JUSTIFY_CENTER | ORANGE;

		if (i < sdata->numGhostProfilesSaved)
		{
			sprintf(rowText, "%s  %s", sdata->ghostProfile_memcard[i].SubmitName_name, DECOMP_RECTMENU_DrawTime(sdata->ghostProfile_memcard[i].trackTime));
		}
		else
		{
			sprintf(rowText, "EMPTY SLOT  %s", DECOMP_RECTMENU_DrawTime(sdata->gGT->drivers[0]->timeElapsedInRace));
		}

		DECOMP_DecalFont_DrawLine(rowText, x + 0x64, y, FONT_SMALL, color);
	}

	DECOMP_DecalFont_DrawLine("X: SAVE   TRIANGLE: BACK", 0x100, 0xa8, FONT_SMALL, JUSTIFY_CENTER | ORANGE);
}

static void SelectProfile_SaveGhostFromRow(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *driver = gGT->drivers[0];
	int time = 0x8c9ff;

	if (driver != NULL)
		time = driver->timeElapsedInRace;

	DECOMP_RefreshCard_GhostEncodeProfile(menu->rowSelected, data.characterIDs[0], gGT->levelID, time, gGT->prevNameEntered);

	sdata->ghostProfile_indexSave = menu->rowSelected;
	sdata->ghostProfile_rowSelect = -1;
	if (menu->rowSelected < sdata->numGhostProfilesSaved)
		sdata->ghostProfile_rowSelect = menu->rowSelected;

	DECOMP_RefreshCard_StartMemcardAction(6);
	*SelectProfile_AllProfiles_MemcardBusy() = 1;
}

static void SelectProfile_DrawOverwriteGhost(struct RectMenu *menu)
{
	s16 width = 0;
	u32 tap;

	SelectProfile_DrawGhostRows(menu, sdata->numGhostProfilesSaved);
	DECOMP_RECTMENU_GetWidth(&data.menuOverwriteGhost, &width, 1);
	DECOMP_RECTMENU_DrawSelf(&data.menuOverwriteGhost, 0, 0, width);

	tap = sdata->buttonTapPerPlayer[0];
	if ((tap & 0x4007f) == 0)
		return;

	if ((tap & BTN_UP) != 0)
	{
		if (data.menuOverwriteGhost.rowSelected > 0)
		{
			// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800491c8-0x800491d4 for overwrite prompt cursor-up SFX.
			DECOMP_OtherFX_Play(0, 1);
			data.menuOverwriteGhost.rowSelected--;
		}
	}
	else if ((tap & BTN_DOWN) != 0)
	{
		if (data.menuOverwriteGhost.rowSelected < 1)
		{
			// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80049204-0x80049210 for overwrite prompt cursor-down SFX.
			DECOMP_OtherFX_Play(0, 1);
			data.menuOverwriteGhost.rowSelected++;
		}
	}
	else if ((tap & (BTN_TRIANGLE | BTN_SQUARE)) != 0)
	{
		// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80049268-0x80049270 for overwrite prompt back SFX.
		DECOMP_OtherFX_Play(2, 1);
		*SelectProfile_AllProfiles_OverwritePrompt() = 0;
	}
	else if ((tap & (BTN_CROSS | BTN_CIRCLE)) != 0)
	{
		// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80049240-0x80049248 for overwrite prompt confirm SFX.
		DECOMP_OtherFX_Play(1, 1);
		if (data.menuOverwriteGhost.rowSelected == 0)
			SelectProfile_SaveGhostFromRow(menu);
		*SelectProfile_AllProfiles_OverwritePrompt() = 0;
	}

	DECOMP_RECTMENU_ClearInput();
}

static void SelectProfile_ExitGhostMenu(struct RectMenu *menu)
{
	DECOMP_RECTMENU_Hide(menu);

	if (sdata->memcardAction == 1)
	{
		*SelectProfile_AllProfiles_ExitToPrevious() = 1;
		sdata->ptrDesiredMenu = &menu224;
		return;
	}

	GhostTape_Destroy();
	sdata->ptrDesiredMenu = DECOMP_MM_TrackSelect_GetMenuPtr();
	DECOMP_MM_TrackSelect_Init();
}

static void SelectProfile_HandleGhostSelection(struct RectMenu *menu, int rowCount)
{
	if (menu->rowSelected < 0)
	{
		SelectProfile_ExitGhostMenu(menu);
		return;
	}

	if (sdata->memcardAction == 1)
	{
		if (menu->rowSelected < sdata->numGhostProfilesSaved)
		{
			data.menuOverwriteGhost.rowSelected = 1;
			*SelectProfile_AllProfiles_OverwritePrompt() = 1;
			sdata->ghostProfile_rowSelect = menu->rowSelected;
			return;
		}

		SelectProfile_SaveGhostFromRow(menu);
		return;
	}

	if (menu->rowSelected >= rowCount - 1)
	{
		*SelectProfile_AllProfiles_MemcardBusy() = 1;
		*SelectProfile_AllProfiles_ExitToPrevious() = 1;
		memset(sdata->ptrGhostTapePlaying, 0, 0x28);
		return;
	}

	if (sdata->ghostProfile_memcard[menu->rowSelected].trackID == sdata->gGT->levelID)
	{
		sdata->ghostProfile_indexLoad = menu->rowSelected;
		DECOMP_RefreshCard_StartMemcardAction(5);
		*SelectProfile_AllProfiles_MemcardBusy() = 1;
		return;
	}

	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800498a8-0x800498b0 for invalid ghost-track SFX.
	DECOMP_OtherFX_Play(5, 1);
}

void DECOMP_SelectProfile_AllProfiles_MenuProc(struct RectMenu *menu)
{
	int rowCount;

	if (*SelectProfile_AllProfiles_Mode() != 0x30)
		return;

	DECOMP_SelectProfile_UnMuteCursors();
	if ((*SelectProfile_AllProfiles_MemcardBusy() != 0) || (*SelectProfile_AllProfiles_OverwritePrompt() != 0))
		DECOMP_SelectProfile_MuteCursors();

	if (sdata->mcStart == 6)
	{
		DECOMP_DecalFont_DrawLine("SAVING GHOST...", 0x100, 0x80, FONT_BIG, JUSTIFY_CENTER | ORANGE);
		return;
	}

	rowCount = SelectProfile_GhostRowCount();

	if (menu->rowSelected < 0)
		menu->rowSelected = 0;

	if ((rowCount > 0) && (menu->rowSelected >= rowCount))
		menu->rowSelected = rowCount - 1;

	if (*SelectProfile_AllProfiles_OverwritePrompt() != 0)
	{
		SelectProfile_DrawOverwriteGhost(menu);
		return;
	}

	SelectProfile_DrawGhostRows(menu, rowCount);

	if (*SelectProfile_AllProfiles_MemcardBusy() != 0)
		return;

	if (DECOMP_SelectProfile_InputLogic(menu, rowCount, 0) != 0)
		SelectProfile_HandleGhostSelection(menu, rowCount);
}
