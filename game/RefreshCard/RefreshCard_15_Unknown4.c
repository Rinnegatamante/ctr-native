#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80047a58-0x80047d64.
void RefreshCard_Unknown4(void)
{
	int result = -1;

	if ((sdata->memcardUnk1 & 1) == 0)
	{
		if (sdata->frame1_memcardAction != 0)
		{
			result = MEMCARD_HandleEvent();
			sdata->frame3_memcardAction = sdata->frame1_memcardAction;
			sdata->frame3_memcardSlot = sdata->frame1_memcardSlot;
		}
	}
	else
	{
		sdata->frame3_memcardAction = sdata->frame1_memcardAction;
		sdata->frame3_memcardSlot = sdata->frame1_memcardSlot;

		sdata->memcardUnk1 &= ~1;
		if ((sdata->memcardUnk1 & 2) == 0)
			sdata->memcardUnk1 &= ~4;

		switch (sdata->frame1_memcardAction)
		{
		case MC_ACTION_GetInfo:
			result = MEMCARD_GetInfo(sdata->frame1_memcardSlot);
			break;
		case MC_ACTION_Save:
			result = MEMCARD_Save(sdata->frame1_memcardSlot, sdata->ghostProfile_fileName, sdata->ghostProfile_fileIconHeader,
			                      (u8 *)sdata->ghostProfile_ptrGhostHeader, sdata->ghostProfile_size3E00, 0);
			break;
		case MC_ACTION_Load:
			result = MEMCARD_Load(sdata->frame1_memcardSlot, sdata->ghostProfile_fileName, (u8 *)sdata->ghostProfile_ptrGhostHeader,
			                      sdata->ghostProfile_size3E00, 0);
			break;
		case MC_ACTION_Format:
			result = MEMCARD_Format(sdata->frame1_memcardSlot);
			break;
		case MC_ACTION_Erase:
			result = MEMCARD_EraseFile(sdata->frame1_memcardSlot, sdata->ghostProfile_fileName);
			break;
		}
	}

	if ((sdata->frame1_memcardAction == MC_ACTION_GetInfo) && (result == MC_RETURN_NEWCARD))
	{
		char *fileName;
		int totalGhosts = 0;

		sdata->numGhostProfilesSaved = 0;
		fileName = MEMCARD_FindFirstGhost(sdata->frame1_memcardSlot, data.s_BASCUS_94426G_Star);

		while (fileName != NULL)
		{
			if (totalGhosts < 7)
			{
				RefreshCard_GhostDecodeProfile(&sdata->ghostProfile_memcard[totalGhosts], fileName);
				sdata->numGhostProfilesSaved++;
			}

			totalGhosts++;
			fileName = MEMCARD_FindNextGhost();
		}

		MEMCARD_IsFile(sdata->frame1_memcardSlot, sdata->ghostProfile_fileName);
		sdata->memcardUnk1 |= 8;
		result = MEMCARD_IsFile(sdata->frame1_memcardSlot, sdata->ghostProfile_fileName);
	}

	switch (result)
	{
	case MC_RETURN_IOE:
		sdata->desired_memcardResult = MC_RESULT_READY_SAVE;
		if (sdata->frame1_memcardAction == MC_ACTION_GetInfo)
		{
			sdata->desired_memcardResult = MC_RESULT_READY_LOAD;
			if ((sdata->memcardUnk1 & 8) == 0)
				sdata->desired_memcardResult = MC_RESULT_READY_SAVE;
		}
		break;
	case MC_RETURN_TIMEOUT:
		sdata->desired_memcardResult = MC_RESULT_ERROR_TIMEOUT;
		break;
	case MC_RETURN_NOCARD:
		sdata->desired_memcardResult = MC_RESULT_ERROR_NOCARD;
		sdata->frame1_memcardAction = 0;
		goto try_next_action;
	case MC_RETURN_NEWCARD:
		sdata->desired_memcardResult = MC_RESULT_NEWCARD;
		if (sdata->frame1_memcardAction == MC_ACTION_Format)
			sdata->desired_memcardResult = MC_RESULT_READY_SAVE;
		break;
	case MC_RETURN_FULL:
		sdata->desired_memcardResult = MC_RESULT_FULL;
		break;
	case MC_RETURN_UNFORMATTED:
		sdata->desired_memcardResult = MC_RESULT_ERROR_UNFORMATTED;
		break;
	case MC_RETURN_NODATA:
		sdata->desired_memcardResult = MC_RESULT_ERROR_NODATA;
		break;
	case MC_RETURN_PENDING:
		sdata->desired_memcardResult = MC_RESULT_PENDING;
		goto try_next_action;
	default:
		goto try_next_action;
	}

	sdata->frame1_memcardAction = 0;

try_next_action:
	if ((sdata->frame1_memcardAction == 0) && (sdata->frame2_memcardAction != 0))
	{
		sdata->frame1_memcardAction = sdata->frame2_memcardAction;
		sdata->frame2_memcardAction = 0;
		sdata->frame1_memcardSlot = sdata->frame2_memcardSlot;
		sdata->memcardUnk1 = (sdata->memcardUnk1 & ~2) | 1;
	}
}
