#include <common.h>

static void RefreshCard_QueueGetInfo(void)
{
	RefreshCard_NextMemcardAction(0, MC_ACTION_GetInfo, data.s_BASCUS_94426_SLOTS, NULL, NULL, 0);
}

static void RefreshCard_QueueMainLoad(void)
{
	RefreshCard_NextMemcardAction(0, MC_ACTION_Load, data.s_BASCUS_94426_SLOTS, NULL, (struct GhostHeader *)sdata->ptrToMemcardBuffer1, 0x1680);
}

static void RefreshCard_QueueMainSave(void)
{
	RefreshCard_NextMemcardAction(0, MC_ACTION_Save, data.s_BASCUS_94426_SLOTS, (char *)data.memcardIcon_HeaderSLOTS,
	                              (struct GhostHeader *)sdata->ptrToMemcardBuffer1, 0x1680);
}

static void RefreshCard_QueueGhostSave(void)
{
	RefreshCard_NextMemcardAction(0, MC_ACTION_Save, data.s_BASCUS_94426G_Question, sdata->memcardIcon_HeaderGHOST, sdata->GhostRecording.ptrGhost, 0x3e00);
}

static void RefreshCard_QueueGhostLoad(void)
{
	RefreshCard_NextMemcardAction(0, MC_ACTION_Load, sdata->ghostProfile_memcard[sdata->ghostProfile_indexLoad].profile_name, NULL, sdata->ptrGhostTapePlaying,
	                              0x3e00);
}

static void RefreshCard_SetScreenAndPoll(int screenText)
{
	RefreshCard_SetScreenText(screenText);
	RefreshCard_QueueGetInfo();
	sdata->boolError = 1;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800472d0-0x80047a58.
void RefreshCard_Unknown3(void)
{
	int keepPolling = 0;

	switch (sdata->mcScreenText)
	{
	case MC_SCREEN_WARNING_NOCARD:
		keepPolling = 1;
		sdata->mcStart = 2;
		sdata->boolError = 1;
		break;

	case MC_SCREEN_WARNING_UNFORMATTED:
		if (sdata->mcStart != 7)
		{
			sdata->mcStart = 2;
			keepPolling = 1;
			sdata->boolError = 1;
			break;
		}

		sdata->mcStart = 2;
		RefreshCard_SetScreenText(MC_SCREEN_FORMATTING);
		RefreshCard_NextMemcardAction(0, MC_ACTION_Format, data.s_BASCUS_94426_SLOTS, NULL, NULL, 0);
		sdata->boolError = 0;
		break;

	case MC_SCREEN_CHECKING:
	case MC_SCREEN_ERROR_FULL:
		keepPolling = 1;
		break;

	case MC_SCREEN_ERROR_TIMEOUT:
	case MC_SCREEN_NULL:
	case MC_SCREEN_ERROR_NODATA:
		if (sdata->mcStart == 5)
		{
			RefreshCard_SetScreenText(MC_SCREEN_LOADING);
			RefreshCard_QueueGhostLoad();
		}
		else if (sdata->mcStart == 3)
		{
			sdata->mcStart = 2;
			RefreshCard_SetScreenText(MC_SCREEN_SAVING);
			RefreshCard_QueueMainSave();
			sdata->boolError = 0;
			break;
		}
		else if (sdata->mcStart == 6)
		{
			if (sdata->ghostProfile_rowSelect >= 0)
			{
				int remaining;

				memcpy(data.s_BASCUS_94426G_Question, sdata->ghostProfile_memcard[sdata->ghostProfile_rowSelect].profile_name,
				       sizeof(data.s_BASCUS_94426G_Question));
				RefreshCard_SetScreenText(MC_SCREEN_SAVING);
				RefreshCard_NextMemcardAction(0, MC_ACTION_Erase, data.s_BASCUS_94426G_Question, NULL, NULL, 0);
				sdata->boolError = 0;

				remaining = (sdata->numGhostProfilesSaved - 1) - sdata->ghostProfile_rowSelect;
				if (remaining != 0)
				{
					memmove(&sdata->ghostProfile_memcard[sdata->ghostProfile_rowSelect], &sdata->ghostProfile_memcard[sdata->ghostProfile_rowSelect + 1],
					        remaining * sizeof(struct GhostProfile));
				}
				sdata->numGhostProfilesSaved--;
				break;
			}

			RefreshCard_SetScreenText(MC_SCREEN_SAVING);
			RefreshCard_QueueGhostSave();
			sdata->boolError = 0;
			break;
		}

		keepPolling = 1;
		sdata->boolError = 1;
		break;
	}

	if (RefreshCard_GetResult(MC_RESULT_NEWCARD) != 0)
	{
		RefreshCard_Unknown2();
		RefreshCard_GetNumGhostsTotal();
		goto done;
	}

	if (RefreshCard_GetResult(MC_RESULT_ERROR_NOCARD) != 0)
	{
		RefreshCard_Unknown2();
		RefreshCard_GetNumGhostsTotal();
		RefreshCard_SetScreenAndPoll(MC_SCREEN_WARNING_NOCARD);
		keepPolling = 0;
		goto done;
	}

	if (RefreshCard_GetResult(MC_RESULT_FULL) != 0)
	{
		RefreshCard_Unknown2();
		RefreshCard_SetScreenAndPoll(MC_SCREEN_ERROR_FULL);
		keepPolling = 0;
		goto done;
	}

	if (RefreshCard_GetResult(MC_RESULT_ERROR_TIMEOUT) != 0)
	{
		RefreshCard_Unknown2();
		RefreshCard_SetScreenAndPoll(MC_SCREEN_ERROR_TIMEOUT);
		keepPolling = 0;
		goto done;
	}

	if (RefreshCard_GetResult(MC_RESULT_ERROR_NODATA) != 0)
	{
		sdata->boolMemcardDataValid = 0;
		RefreshCard_Unknown2();
		sdata->boolError = 1;

		if ((sdata->memcardAction >= 0) && (sdata->memcardAction < 3))
		{
			RefreshCard_SetScreenText(MC_SCREEN_ERROR_NODATA);
			RefreshCard_QueueGetInfo();
			keepPolling = 0;
		}
		goto done;
	}

	if (RefreshCard_GetResult(MC_RESULT_READY_LOAD) != 0)
	{
		RefreshCard_Unknown2();
		*(s16 *)&sdata->unk8008d95c = 0;
		*(s16 *)&sdata->boolAdvProfilesChecked = 0;
		RefreshCard_SetScreenText(MC_SCREEN_LOADING);
		RefreshCard_QueueMainLoad();
		sdata->boolError = 0;
		keepPolling = 0;
		goto done;
	}

	if (RefreshCard_GetResult(MC_RESULT_ERROR_UNFORMATTED) != 0)
	{
		RefreshCard_GetNumGhostsTotal();
		RefreshCard_Unknown2();
		RefreshCard_SetScreenAndPoll(MC_SCREEN_WARNING_UNFORMATTED);
		keepPolling = 0;
		goto done;
	}

	if (RefreshCard_GetResult(MC_RESULT_READY_SAVE) == 0)
		goto done;

	sdata->boolError = 1;

	if (sdata->mcScreenText == MC_SCREEN_SAVING)
	{
		if (sdata->mcStart == 6)
		{
			if (sdata->ghostProfile_rowSelect >= 0)
			{
				sdata->ghostProfile_rowSelect = -1;
				RefreshCard_SetScreenText(MC_SCREEN_SAVING);
				RefreshCard_QueueGhostSave();
				sdata->boolError = 0;
				keepPolling = 0;
				goto done;
			}

			{
				int remaining = (sdata->numGhostProfilesSaved - 1) - sdata->ghostProfile_indexSave;

				if (remaining != 0)
				{
					memmove(&sdata->ghostProfile_memcard[sdata->ghostProfile_indexSave + 1], &sdata->ghostProfile_memcard[sdata->ghostProfile_indexSave],
					        remaining * sizeof(struct GhostProfile));
				}

				sdata->numGhostProfilesSaved++;
				sdata->ghostProfile_memcard[sdata->ghostProfile_indexSave] = sdata->ghostProfile_current;
			}
		}

		*(s16 *)&sdata->unk8008d964 = 1;
		sdata->mcStart = 2;
		RefreshCard_SetScreenText(MC_SCREEN_NULL);
		RefreshCard_QueueGetInfo();
		keepPolling = 0;
		goto done;
	}

	if (sdata->mcScreenText < MC_SCREEN_LOADING)
	{
		if (sdata->mcScreenText == MC_SCREEN_FORMATTING)
		{
			RefreshCard_GetNumGhostsTotal();
			RefreshCard_Unknown2();
			*(s16 *)&sdata->unk_memcardRelated_8008d928[0] = 1;

			if (sdata->memcardAction >= 0)
			{
				if (sdata->memcardAction < 2)
				{
					GAMEPROG_InitFullMemcard(sdata->ptrToMemcardBuffer2);
					RefreshCard_SetScreenText(MC_SCREEN_NULL);
					RefreshCard_QueueGetInfo();
					keepPolling = 0;
				}
				else if (sdata->memcardAction == 2)
				{
					sdata->boolError = 1;
				}
			}
		}
		goto done;
	}

	if (sdata->mcScreenText != MC_SCREEN_LOADING)
	{
		if (sdata->mcScreenText == MC_SCREEN_CHECKING)
		{
			RefreshCard_GetNumGhostsTotal();
			RefreshCard_Unknown2();
			*(s16 *)&sdata->unk8008d95c = 0;
			*(s16 *)&sdata->boolAdvProfilesChecked = 0;
			RefreshCard_SetScreenText(MC_SCREEN_LOADING);
			RefreshCard_QueueMainLoad();
			sdata->boolError = 0;
			keepPolling = 0;
		}
		goto done;
	}

	if (sdata->mcStart == 5)
	{
		sdata->boolReplayHumanGhost = 1;
		*(s16 *)&sdata->unk8008d964 = 1;
		RefreshCard_SetScreenText(MC_SCREEN_NULL);
	}
	else if (*(int *)sdata->ptrToMemcardBuffer2 == 0x1600ffee)
	{
		sdata->boolMemcardDataValid = 0;
		RefreshCard_GameProgressAndOptions();
		RefreshCard_SetScreenText(MC_SCREEN_NULL);
	}
	else
	{
		sdata->boolMemcardDataValid = 1;
		RefreshCard_Unknown2();
		RefreshCard_SetScreenText(MC_SCREEN_ERROR_NODATA);
	}

	sdata->mcStart = 2;
	RefreshCard_QueueGetInfo();
	keepPolling = 0;

done:
	if ((keepPolling != 0) && (RefreshCard_GetResult(MC_RESULT_PENDING) == 0))
		RefreshCard_QueueGetInfo();
}
