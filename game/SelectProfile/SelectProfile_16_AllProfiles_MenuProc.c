#include <common.h>

extern struct RectMenu menu224;
extern struct RectMenu menu224NoSave;

static s16 *SelectProfile_AllProfiles_Mode(void)
{
	return (s16 *)&sdata->data10_bbb[0];
}

static s16 *SelectProfile_AllProfiles_ActionActive(void)
{
	return (s16 *)&sdata->data10_bbb[2];
}

static s16 *SelectProfile_AllProfiles_ExitToPrevious(void)
{
	return (s16 *)&sdata->data10_bbb[4];
}

static s16 *SelectProfile_AllProfiles_ActionDone(void)
{
	return (s16 *)&sdata->data10_bbb[6];
}

static s16 *SelectProfile_AllProfiles_OverwritePrompt(void)
{
	return (s16 *)&sdata->data10_bbb[8];
}

static s16 *SelectProfile_AllProfiles_TimeoutPrompt(void)
{
	return (s16 *)&sdata->data10_bbb[10];
}

static s16 *SelectProfile_AllProfiles_TimerSaveComplete(void)
{
	return (s16 *)&sdata->data10_bbb[12];
}

static struct MemcardProfile *SelectProfile_MemcardProfile(void)
{
	return (struct MemcardProfile *)sdata->ptrToMemcardBuffer2;
}

static int SelectProfile_IsGhostMode(void)
{
	return *SelectProfile_AllProfiles_Mode() == 0x30;
}

static int SelectProfile_AdvProfileOccupied(int slot)
{
	return SelectProfile_MemcardProfile()->advProgress[slot].characterID >= 0;
}

static void SelectProfile_CopyGameProgressToCard(void)
{
	struct MemcardProfile *memcard = SelectProfile_MemcardProfile();

	RaceConfig_SaveGameOptions();
	GAMEPROG_SaveCupProgress();
	GAMEPROG_SyncGameAndCard(&memcard->gameProgress, &sdata->gameProgress);
	memcpy(&memcard->gameProgress, &sdata->gameProgress, sizeof(struct GameProgress) + sizeof(struct GameOptions));
}

static void SelectProfile_LoadAdvProfile(int slot)
{
	struct GameTracker *gGT = sdata->gGT;
	struct MemcardProfile *memcard = SelectProfile_MemcardProfile();

	GAMEPROG_SyncGameAndCard(&memcard->gameProgress, &sdata->gameProgress);
	sdata->advProgress = memcard->advProgress[slot];
	data.characterIDs[0] = sdata->advProgress.characterID;
	memmove(gGT->prevNameEntered, sdata->advProgress.name, sizeof(gGT->prevNameEntered));
}

static void SelectProfile_SaveAdvProfile(int slot)
{
	struct MemcardProfile *memcard = SelectProfile_MemcardProfile();

	sdata->unk_8008d73C_relatedToRowHighlighted = slot;
	SelectProfile_CopyGameProgressToCard();
	memcard->advProgress[slot] = sdata->advProgress;
	MEMCARD_SetIcon(0);
	RefreshCard_StartMemcardAction(3);
	*(s16 *)&sdata->unk_memcardRelated_8008d928[0] = 1;
	*SelectProfile_AllProfiles_ActionActive() = 1;
}

static void SelectProfile_StartGhostSave(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *driver = gGT->drivers[0];
	int time = 0x8ca00;

	if (driver != NULL)
		time = driver->timeElapsedInRace;

	RefreshCard_GhostEncodeProfile(menu->rowSelected, data.characterIDs[0], gGT->levelID, time, gGT->prevNameEntered);

	sdata->ghostProfile_indexSave = menu->rowSelected;
	sdata->ghostProfile_rowSelect = -1;
	if (menu->rowSelected < sdata->numGhostProfilesSaved)
		sdata->ghostProfile_rowSelect = menu->rowSelected;

	MEMCARD_SetIcon(1);
	RefreshCard_StartMemcardAction(6);
	*SelectProfile_AllProfiles_ActionActive() = 1;
	gGT->gameModeEnd |= PLAYER_GHOST_BEAT;
}

static int SelectProfile_GhostRowCount(int *savedCount, int *canChooseEmptySlot)
{
	int count = sdata->numGhostProfilesSaved;

	if (count < 0)
		count = 0;

	if (count > 7)
		count = 7;

	*savedCount = count;

	if (sdata->memcardAction == 1)
	{
		*canChooseEmptySlot = sdata->memoryCard_SizeRemaining >= 0x3e00;
		count += *canChooseEmptySlot;

		if (count > 7)
		{
			count = 7;
			*canChooseEmptySlot = 0;
		}
	}
	else
	{
		*canChooseEmptySlot = 1;
		count++;
	}

	return count;
}

static void SelectProfile_ClampRow(struct RectMenu *menu, int rowCount)
{
	if (menu->rowSelected < 0)
		menu->rowSelected = 0;

	if ((rowCount > 0) && (menu->rowSelected >= rowCount))
		menu->rowSelected = rowCount - 1;
}

static int SelectProfile_DisableAdvInputFlags(void)
{
	int flags = 0;

	if ((sdata->mcScreenText < MC_SCREEN_FORMATTING) ||
	    ((sdata->memoryCard_SizeRemaining < 0x1680) && (*(s16 *)&sdata->unk_memcardRelated_8008d928[0] == 0) && (sdata->memcardAction == 1)))
		flags = 1;

	if (sdata->mcScreenText < MC_SCREEN_FORMATTING)
		flags |= 2;

	return flags;
}

static void SelectProfile_DrawGhostRows(struct RectMenu *menu, int rowCount, int savedCount, int canChooseEmptySlot, int color)
{
	int i;
	int lineGap;
	int yBase;
	int subtitleVisible;
	int rowCountWithEmpty = sdata->numGhostProfilesSaved + canChooseEmptySlot;
	struct GhostProfile *profile = &sdata->ghostProfile_memcard[0];

	subtitleVisible = strlen(sdata->lngStrings[data.lngIndex_LoadSave[(sdata->memcardAction * 2) + 1]]) != 0;

	if (rowCount < 7)
	{
		lineGap = 0x10;
		yBase = 0x12;
		if (sdata->memcardAction != 1)
			DecalFont_DrawMultiLine(sdata->lngStrings[0xcf], 0x100, 0xbe, 0x1ce, FONT_SMALL, color | 0xffff8000);
	}
	else
	{
		lineGap = 8;
		yBase = subtitleVisible ? 0xc : 0x12;
	}

	DecalFont_DrawLine(sdata->lngStrings[data.lngIndex_LoadSave[sdata->memcardAction * 2]], 0x100, yBase, rowCount < 7 ? FONT_BIG : FONT_SMALL,
	                   JUSTIFY_CENTER | color);

	if (subtitleVisible != 0)
	{
		DecalFont_DrawLine(sdata->lngStrings[data.lngIndex_LoadSave[(sdata->memcardAction * 2) + 1]], 0x100, yBase + lineGap,
		                   rowCount < 7 ? FONT_BIG : FONT_SMALL, JUSTIFY_CENTER | color);
	}

	for (i = 0; i < rowCountWithEmpty; i++)
	{
		int pair = i >> 1;
		int x;
		int y;
		int drawStyle = menu->drawStyle;
		int isWrongTrack = 0;

		if ((i == savedCount) && (i < rowCountWithEmpty))
			profile = NULL;

		if ((i < rowCount - 1) || ((i & 1) != 0))
			x = ((i & 1) * 0xd4) + 0x2e;
		else
			x = 0x98;

		y = yBase + lineGap + 6 + (pair * ((rowCount > 6) ? 0x2b : 0x2f));

		if (rowCount > 6)
			drawStyle |= 0x40;

		if ((profile != NULL) && (profile->trackID != sdata->gGT->levelID))
			isWrongTrack = sdata->memcardAction != 1;

		SelectProfile_DrawGhostProfile(profile, x, y, i == menu->rowSelected, i, drawStyle, sdata->memcardAction == 0, isWrongTrack);

		if (profile != NULL)
			profile++;
	}
}

static void SelectProfile_DrawAdvRows(struct RectMenu *menu, int color)
{
	int i;
	int subtitleVisible = strlen(sdata->lngStrings[data.lngStringsSaveLoadDelete[(sdata->memcardAction * 2) + 1]]) != 0;
	struct MemcardProfile *memcard = SelectProfile_MemcardProfile();

	DecalFont_DrawLine(sdata->lngStrings[data.lngStringsSaveLoadDelete[sdata->memcardAction * 2]], 0x100, subtitleVisible ? 0x12 : 0x1a, FONT_BIG,
	                   JUSTIFY_CENTER | color);

	if (subtitleVisible != 0)
	{
		DecalFont_DrawLine(sdata->lngStrings[data.lngStringsSaveLoadDelete[(sdata->memcardAction * 2) + 1]], 0x100, 0x22, FONT_BIG, JUSTIFY_CENTER | color);
	}

	for (i = 0; i < 4; i++)
	{
		SelectProfile_DrawAdvProfile(&memcard->advProgress[i], ((i & 1) * 0xea) + 0x1a, ((i >> 1) * 0x43) + 0x3c, i == menu->rowSelected, i, menu->drawStyle);
	}

	if ((sdata->memcardAction == 1) && (sdata->boolMemcardDataValid != 0))
	{
		DecalFont_DrawLine(sdata->lngStrings[0xd0], 0x100, 0xc3, FONT_SMALL, JUSTIFY_CENTER | RED);
	}
}

static void SelectProfile_DrawOverwriteMenu(struct RectMenu *menu)
{
	s16 width = 0;
	struct RectMenu *overwriteMenu = SelectProfile_IsGhostMode() ? &data.menuOverwriteGhost : &data.menuOverwriteAdv;

	RECTMENU_GetWidth(overwriteMenu, &width, 1);
	RECTMENU_DrawSelf(overwriteMenu, 0, 0, width);

	if (SelectProfile_IsGhostMode())
	{
		SelectProfile_DrawGhostProfile(&sdata->ghostProfile_memcard[sdata->ghostProfile_rowSelect], 0x9c, 0x3c, 0, 0, menu->drawStyle, 0, 0);
	}
	else
	{
		SelectProfile_DrawAdvProfile(&SelectProfile_MemcardProfile()->advProgress[menu->rowSelected], 0x92, 0x3c, 0, menu->rowSelected, menu->drawStyle);
	}
}

static int SelectProfile_ProcessOverwritePrompt(void)
{
	struct RectMenu *overwriteMenu = &data.menuOverwriteAdv;
	u32 tap = sdata->buttonTapPerPlayer[0];
	int confirm = 0;

	if ((tap & 0x4007f) == 0)
		return 0;

	if ((tap & BTN_UP) != 0)
	{
		if (overwriteMenu->rowSelected > 0)
		{
			OtherFX_Play(0, 1);
			overwriteMenu->rowSelected--;
		}
	}
	else if ((tap & BTN_DOWN) != 0)
	{
		if (overwriteMenu->rowSelected < 1)
		{
			OtherFX_Play(0, 1);
			overwriteMenu->rowSelected++;
		}
	}
	else if ((tap & (BTN_CROSS | BTN_CIRCLE | BTN_TRIANGLE | BTN_SQUARE)) != 0)
	{
		if ((tap & (BTN_CROSS | BTN_CIRCLE)) != 0)
		{
			OtherFX_Play(1, 1);
			confirm = overwriteMenu->rowSelected == 0;
		}
		else
		{
			OtherFX_Play(2, 1);
		}
		*SelectProfile_AllProfiles_OverwritePrompt() = 0;
	}

	RECTMENU_ClearInput();
	data.menuOverwriteAdv.rowSelected = overwriteMenu->rowSelected;
	data.menuOverwriteGhost.rowSelected = overwriteMenu->rowSelected;
	return confirm;
}

static void SelectProfile_HandleNoCardOrSpace(struct RectMenu *menu)
{
	int mode = *SelectProfile_AllProfiles_Mode();

	if (sdata->mcScreenText == MC_SCREEN_WARNING_NOCARD)
	{
		if (sdata->memcardAction == 1)
		{
			*SelectProfile_AllProfiles_ActionActive() = sdata->memcardAction;
			*SelectProfile_AllProfiles_ActionDone() = sdata->memcardAction;
			sdata->boolError = 1;
		}
		else if ((sdata->memcardAction == 0) && (mode == 0x30))
		{
			*SelectProfile_AllProfiles_ActionActive() = 1;
			*SelectProfile_AllProfiles_ActionDone() = 1;
			sdata->boolError = 1;
		}
	}

	if ((sdata->mcScreenText != MC_SCREEN_WARNING_UNFORMATTED) && (sdata->memoryCard_SizeRemaining < 0x1680) &&
	    (*(s16 *)&sdata->unk_memcardRelated_8008d928[0] == 0) && (sdata->memcardAction == 1))
	{
		*SelectProfile_AllProfiles_ActionActive() = 1;
		*SelectProfile_AllProfiles_ActionDone() = 1;
		sdata->boolError = 1;
	}

	if (menu->rowSelected == -1)
	{
		*SelectProfile_AllProfiles_ActionActive() = 1;
		*SelectProfile_AllProfiles_ExitToPrevious() = 1;
		sdata->boolError = 1;
	}
}

static void SelectProfile_StartLoadGhost(struct RectMenu *menu, int rowCount)
{
	if (menu->rowSelected >= rowCount - 1)
	{
		if (sdata->ptrGhostTapePlaying != NULL)
			memset(sdata->ptrGhostTapePlaying, 0, 0x28);

		*SelectProfile_AllProfiles_ActionActive() = 1;
		*SelectProfile_AllProfiles_ActionDone() = 1;
		sdata->boolError = 1;
		return;
	}

	if (sdata->ghostProfile_memcard[menu->rowSelected].trackID == sdata->gGT->levelID)
	{
		sdata->ghostProfile_indexLoad = menu->rowSelected;
		RefreshCard_StartMemcardAction(5);
		*SelectProfile_AllProfiles_ActionActive() = 1;
		return;
	}

	OtherFX_Play(5, 1);
}

static int SelectProfile_HandleSelection(struct RectMenu *menu, int rowCount)
{
	int mode = *SelectProfile_AllProfiles_Mode();

	if (menu->rowSelected == -1)
	{
		*SelectProfile_AllProfiles_ActionActive() = 1;
		*SelectProfile_AllProfiles_ExitToPrevious() = 1;
		sdata->boolError = 1;
		return 0;
	}

	if (sdata->mcScreenText == MC_SCREEN_WARNING_UNFORMATTED)
	{
		RefreshCard_StartMemcardAction(7);
		return 0;
	}

	if ((*(s16 *)&sdata->unk8008d95c == 0) && (*(s16 *)&sdata->unk_memcardRelated_8008d928[0] == 0))
		return 0;

	if (sdata->memcardAction == 1)
	{
		if (mode == 0x30)
		{
			if (menu->rowSelected < sdata->numGhostProfilesSaved)
			{
				data.menuOverwriteAdv.rowSelected = 1;
				data.menuOverwriteGhost.rowSelected = 1;
				*SelectProfile_AllProfiles_OverwritePrompt() = 1;
				sdata->ghostProfile_rowSelect = menu->rowSelected;
				return 0;
			}
		}
		else
		{
			if ((*SelectProfile_AllProfiles_TimeoutPrompt() == 0) && (sdata->mcScreenText == MC_SCREEN_ERROR_TIMEOUT))
			{
				*SelectProfile_AllProfiles_TimeoutPrompt() = 1;
				return 0;
			}

			if (SelectProfile_AdvProfileOccupied(menu->rowSelected) && (menu->rowSelected != sdata->advProfileIndex))
			{
				data.menuOverwriteAdv.rowSelected = 1;
				data.menuOverwriteGhost.rowSelected = 1;
				*SelectProfile_AllProfiles_OverwritePrompt() = 1;
				return 0;
			}
		}

		return 1;
	}

	if (sdata->memcardAction == 0)
	{
		if (mode == 0x30)
		{
			SelectProfile_StartLoadGhost(menu, rowCount);
		}
		else if (sdata->mcScreenText == MC_SCREEN_ERROR_TIMEOUT)
		{
			*SelectProfile_AllProfiles_ActionActive() = 1;
			*SelectProfile_AllProfiles_ExitToPrevious() = 1;
		}
		else if (SelectProfile_AdvProfileOccupied(menu->rowSelected))
		{
			SelectProfile_LoadAdvProfile(menu->rowSelected);
			sdata->unk_8008d73C_relatedToRowHighlighted = menu->rowSelected;
			*SelectProfile_AllProfiles_ActionActive() = 1;
			*SelectProfile_AllProfiles_ActionDone() = 1;
			sdata->boolError = 1;
		}
		else
		{
			OtherFX_Play(5, 1);
		}
	}
	else if ((sdata->memcardAction == 2) && (mode != 0x30) && SelectProfile_AdvProfileOccupied(menu->rowSelected))
	{
		GAMEPROG_NewProfile_InsideAdv(&SelectProfile_MemcardProfile()->advProgress[menu->rowSelected]);
		MEMCARD_SetIcon(0);
		RefreshCard_StartMemcardAction(3);
		*(s16 *)&sdata->unk_memcardRelated_8008d928[0] = 1;
		*SelectProfile_AllProfiles_ActionActive() = 1;
	}

	return 0;
}

static void SelectProfile_DrawMemcardMessage(int screen, int color, int menuFlag)
{
	int descriptor;
	int firstString;
	int multiLine;
	int i;

	if ((screen < 0) || (screen >= 10))
		return;

	if ((screen == MC_SCREEN_ERROR_NODATA) && (*SelectProfile_AllProfiles_Mode() == 0x40))
		return;

	descriptor = data.messageScreens[screen];
	firstString = descriptor & 0xffff;
	multiLine = (descriptor >> 16) & 0xffff;

	if ((*SelectProfile_AllProfiles_ActionActive() != 0) && (*(s16 *)&sdata->unk8008d964 != 0))
		firstString = 0xffff;

	if (firstString >= 0xffff)
		return;

	if ((firstString == 0x10f) && (sdata->memcardAction == 1))
		firstString = 0x106;

	if ((sdata->memcardAction == 2) && (firstString == 0xea))
		firstString = 0xfc;

	if (multiLine == 0)
	{
		DecalFont_DrawLine(sdata->lngStrings[firstString], 0x108, 0x12, FONT_BIG, JUSTIFY_CENTER | color);
	}
	else
	{
		for (i = 0; i < 9; i++)
		{
			char *line = sdata->lngStrings[firstString + i];

			if (strlen(line) != 0)
			{
				int font = (i == 0) ? FONT_BIG : FONT_SMALL;
				int y = (i == 0) ? 0x26 : 0x2e + (i * 10);
				int lineColor = color | 0xffff8000;

				if (((sdata->frameCounter & 4) == 0) && (i == 0))
					lineColor = RED | 0xffff8000;

				DecalFont_DrawLine(line, 0x100, y, font, lineColor);
			}
		}
	}

	RECTMENU_DrawInnerRect((RECT *)&sdata->unk_BeforeTokenMenu[0], menuFlag, sdata->gGT->backBuffer->otMem.startPlusFour);
}

static void SelectProfile_DrawAll(struct RectMenu *menu, int rowCount, int savedGhostCount, int canChooseEmptySlot, int color)
{
	int canDrawProfiles =
	    (*SelectProfile_AllProfiles_ActionActive() == 0) &&
	    ((*(s16 *)&sdata->unk8008d95c != 0) || (*(s16 *)&sdata->unk_memcardRelated_8008d928[0] != 0) || (sdata->mcScreenText == MC_SCREEN_NULL));

	if ((sdata->memcardAction == 0) && SelectProfile_IsGhostMode() &&
	    ((sdata->mcScreenText == MC_SCREEN_ERROR_NODATA) || (sdata->mcScreenText == MC_SCREEN_WARNING_NOCARD)) && (rowCount != 0))
		canDrawProfiles = 1;

	if (sdata->memcardAction == 1)
	{
		if ((sdata->mcScreenText == MC_SCREEN_NULL) || (sdata->mcScreenText == MC_SCREEN_ERROR_NODATA))
		{
			canDrawProfiles = SelectProfile_IsGhostMode()
			                      ? !((sdata->memoryCard_SizeRemaining < 0x3e00) && (savedGhostCount == 0))
			                      : !((sdata->memoryCard_SizeRemaining < 0x1680) && (*(s16 *)&sdata->unk_memcardRelated_8008d928[0] == 0));
		}

		if ((sdata->mcScreenText == MC_SCREEN_ERROR_TIMEOUT) && (*SelectProfile_AllProfiles_TimeoutPrompt() != 0))
			canDrawProfiles = 1;
	}

	SelectProfile_Init(menu->drawStyle);

	if (canDrawProfiles && (*SelectProfile_AllProfiles_ActionActive() == 0))
	{
		if (SelectProfile_IsGhostMode())
		{
			if (*SelectProfile_AllProfiles_OverwritePrompt() == 0)
				SelectProfile_DrawGhostRows(menu, rowCount, savedGhostCount, canChooseEmptySlot, color);
			else
				SelectProfile_DrawOverwriteMenu(menu);
		}
		else
		{
			if (*SelectProfile_AllProfiles_OverwritePrompt() == 0)
				SelectProfile_DrawAdvRows(menu, color);
			else
				SelectProfile_DrawOverwriteMenu(menu);
		}
	}
	else
	{
		*SelectProfile_AllProfiles_OverwritePrompt() = 0;

		if ((*SelectProfile_AllProfiles_ActionActive() != 0) && (*(s16 *)&sdata->unk8008d964 != 0) && (*SelectProfile_AllProfiles_ExitToPrevious() == 0) &&
		    (*SelectProfile_AllProfiles_ActionDone() == 0) && (*SelectProfile_AllProfiles_TimerSaveComplete() != 0))
		{
			int saveColor = ((sdata->frameCounter & 4) == 0) ? (JUSTIFY_CENTER | ORANGE) : (JUSTIFY_CENTER | LIGHT_GREEN);
			DecalFont_DrawLine(sdata->lngStrings[0x13d], 0x108, 0x64, FONT_BIG, saveColor);
		}
		else
		{
			SelectProfile_DrawMemcardMessage(sdata->mcScreenText, color, menu->drawStyle);
		}
	}
}

static int SelectProfile_ShouldFinalize(void)
{
	if (*SelectProfile_AllProfiles_ActionActive() == 0)
		return 0;

	if (sdata->boolError == 0)
		return 0;

	if ((*(s16 *)&sdata->unk8008d964 == 0) && (*SelectProfile_AllProfiles_ExitToPrevious() == 0) && (*SelectProfile_AllProfiles_ActionDone() == 0))
		return 0;

	if ((*SelectProfile_AllProfiles_ActionActive() != 0) && (*(s16 *)&sdata->unk8008d964 != 0) && (*SelectProfile_AllProfiles_ExitToPrevious() == 0) &&
	    (*SelectProfile_AllProfiles_ActionDone() == 0) && (*SelectProfile_AllProfiles_TimerSaveComplete() != 0))
	{
		(*SelectProfile_AllProfiles_TimerSaveComplete())--;
		return 0;
	}

	return 1;
}

static void SelectProfile_FinalizeGhost(struct RectMenu *menu)
{
	SelectProfile_Destroy();

	if (sdata->memcardAction == 1)
	{
		sdata->ptrDesiredMenu = (*SelectProfile_AllProfiles_ExitToPrevious() != 0) ? &menu224 : &menu224NoSave;
		return;
	}

	if (*SelectProfile_AllProfiles_ExitToPrevious() != 0)
	{
		GhostTape_Destroy();
		sdata->ptrDesiredMenu = MM_TrackSelect_GetMenuPtr();
		MM_TrackSelect_Init();
		return;
	}

	if (sdata->ptrGhostTapePlaying != NULL)
		data.characterIDs[1] = sdata->ptrGhostTapePlaying->characterID;

	sdata->ptrDesiredMenu = QueueLoadTrack_GetMenuPtr();
	(void)menu;
}

static void SelectProfile_FinalizeAdventure(struct RectMenu *menu)
{
	int mode = *SelectProfile_AllProfiles_Mode();
	struct GameTracker *gGT = sdata->gGT;

	if (mode == 0x20)
	{
		if ((*SelectProfile_AllProfiles_ExitToPrevious() == 0) && (sdata->memcardAction == 0))
		{
			sdata->advProfileIndex = menu->rowSelected;
			GAMEPROG_AdvPercent(&sdata->advProgress);
			sdata->ptrDesiredMenu = &data.menuQueueLoadHub;
			gGT->levelID = sdata->advProgress.HubLevYouSavedOn;
			data.menuQueueLoadHub.rowSelected = 3;
		}
		else
		{
			sdata->ptrDesiredMenu = &data.menuGreenLoadSave;
			data.menuGreenLoadSave.rowSelected = 3;
		}
		return;
	}

	if (mode == 0)
	{
		if (*SelectProfile_AllProfiles_ExitToPrevious() != 0)
		{
			sdata->ptrDesiredMenu = &data.menuSubmitName;
			SubmitName_RestoreName(0);
			return;
		}

		sdata->advProfileIndex = menu->rowSelected;
		gGT->levelID = 0x1a;
		Garage_Leave();
		return;
	}

	if (mode == 0x10)
	{
		if (*SelectProfile_AllProfiles_ExitToPrevious() != 0)
		{
			struct RectMenu *advMenu;

			MM_JumpTo_Title_Returning();
			advMenu = MM_AdvNewLoad_GetMenuPtr();
			advMenu->state &= ~4;
			return;
		}

		sdata->advProfileIndex = menu->rowSelected;
		if (sdata->advProgress.HubLevYouSavedOn != 0)
			gGT->levelID = sdata->advProgress.HubLevYouSavedOn;
		else
			gGT->levelID = 0x1a;
		memmove(gGT->prevNameEntered, sdata->advProgress.name, sizeof(gGT->prevNameEntered));
		memmove(gGT->currNameEntered, sdata->advProgress.name, sizeof(gGT->currNameEntered));
		sdata->ptrDesiredMenu = QueueLoadTrack_GetMenuPtr();
		return;
	}

	if (mode == 0x40)
	{
		SelectProfile_Destroy();
		if (sdata->boolSaveCupProgress == 0)
		{
			gGT->gameModeEnd &= ~(NEW_NAME | NEW_HIGH_SCORE);
		}
		else if (*SelectProfile_AllProfiles_ExitToPrevious() != 0)
		{
			sdata->ptrDesiredMenu = &data.menuWarning2;
			return;
		}

		RECTMENU_Hide(menu);
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800490c4-0x8004aa08.
void SelectProfile_AllProfiles_MenuProc(struct RectMenu *menu)
{
	int color = ((menu->drawStyle & 0x10) != 0) ? DARK_RED : ORANGE;
	int savedGhostCount = sdata->numGhostProfilesSaved;
	int canChooseEmptySlot = 0;
	int rowCount = SelectProfile_IsGhostMode() ? SelectProfile_GhostRowCount(&savedGhostCount, &canChooseEmptySlot) : 4;
	int handled = 0;
	int doSave = 0;

	if (sdata->mcScreenText == MC_SCREEN_WARNING_NOCARD)
		*SelectProfile_AllProfiles_OverwritePrompt() = 0;

	if (sdata->mcScreenText < MC_SCREEN_FORMATTING)
		*SelectProfile_AllProfiles_ActionActive() = 0;

	SelectProfile_UnMuteCursors();
	if ((*SelectProfile_AllProfiles_ExitToPrevious() != 0) || (*SelectProfile_AllProfiles_ActionDone() != 0) ||
	    (*SelectProfile_AllProfiles_ActionActive() != 0))
	{
		SelectProfile_MuteCursors();
	}

	if (*SelectProfile_AllProfiles_OverwritePrompt() != 0)
	{
		doSave = SelectProfile_ProcessOverwritePrompt();
		goto draw_and_finish;
	}

	if (SelectProfile_IsGhostMode())
		SelectProfile_ClampRow(menu, rowCount);

	if (*SelectProfile_AllProfiles_ActionActive() == 0)
	{
		if (SelectProfile_IsGhostMode())
		{
			handled = SelectProfile_InputLogic(menu, rowCount, 0);
		}
		else if (*SelectProfile_AllProfiles_Mode() == 0x40)
		{
			if (sdata->mcScreenText == MC_SCREEN_WARNING_NOCARD)
			{
				if ((sdata->buttonTapPerPlayer[0] & (BTN_CROSS | BTN_CIRCLE)) != 0)
				{
					OtherFX_Play(1, 1);
					if (sdata->boolSaveCupProgress == 0)
						MainGameEnd_SoloRaceSaveHighScore();
					RECTMENU_ClearInput();
					*SelectProfile_AllProfiles_ActionActive() = 1;
					*SelectProfile_AllProfiles_ExitToPrevious() = 1;
					sdata->boolError = 1;
					handled = 0;
				}
			}
			else if (sdata->mcScreenText == MC_SCREEN_WARNING_UNFORMATTED)
			{
				if ((sdata->buttonTapPerPlayer[0] & BTN_CIRCLE) != 0)
				{
					RECTMENU_ClearInput();
					OtherFX_Play(1, 1);
					RefreshCard_StartMemcardAction(7);
				}
			}
			else if (((sdata->memoryCard_SizeRemaining >= 0x1680) || (*(s16 *)&sdata->unk_memcardRelated_8008d928[0] != 0) || (sdata->memcardAction != 1)) &&
			         ((*(s16 *)&sdata->unk8008d95c != 0) || (*(s16 *)&sdata->unk_memcardRelated_8008d928[0] != 0)))
			{
				if (sdata->boolSaveCupProgress == 0)
					MainGameEnd_SoloRaceSaveHighScore();

				SelectProfile_CopyGameProgressToCard();
				MEMCARD_SetIcon(0);
				RefreshCard_StartMemcardAction(3);
				*(s16 *)&sdata->unk_memcardRelated_8008d928[0] = 1;
				*SelectProfile_AllProfiles_ActionActive() = 1;
				handled = 0;
			}
		}
		else
		{
			handled = SelectProfile_InputLogic(menu, rowCount, SelectProfile_DisableAdvInputFlags());
		}

		if (handled != 0)
		{
			SelectProfile_HandleNoCardOrSpace(menu);
			doSave = SelectProfile_HandleSelection(menu, rowCount);
		}
	}

draw_and_finish:
	if ((sdata->mcScreenText == MC_SCREEN_ERROR_TIMEOUT) && (*SelectProfile_AllProfiles_ExitToPrevious() == 0) &&
	    (*SelectProfile_AllProfiles_ActionDone() == 0))
	{
		*SelectProfile_AllProfiles_ActionActive() = 0;
		*SelectProfile_AllProfiles_OverwritePrompt() = 0;
	}

	if (doSave != 0)
	{
		*SelectProfile_AllProfiles_TimeoutPrompt() = 0;
		if (SelectProfile_IsGhostMode())
			SelectProfile_StartGhostSave(menu);
		else
			SelectProfile_SaveAdvProfile(menu->rowSelected);

		*SelectProfile_AllProfiles_TimerSaveComplete() = 0x3c;
	}

	if (menu->unk1e == 1)
		SelectProfile_DrawAll(menu, rowCount, savedGhostCount, canChooseEmptySlot, color);

	if (SelectProfile_ShouldFinalize())
	{
		SelectProfile_InitAndDestroy();
		RefreshCard_StopMemcardAction();

		if (*SelectProfile_AllProfiles_Mode() == 0x30)
			SelectProfile_FinalizeGhost(menu);
		else
			SelectProfile_FinalizeAdventure(menu);
	}
}
