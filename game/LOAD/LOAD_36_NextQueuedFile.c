#include <common.h>

void LOAD_NextQueuedFile()
{
	// Under 3 conditions, delay the load:
	//		During XA play
	//		Queue is in use
	//		Queue is empty
	if (sdata->XA_State != 0)
		return;
	if (sdata->queueReady == 0)
		return;
	if (sdata->queueLength == 0)
		return;


	sdata->queueReady = 0;

	struct LoadQueueSlot *curr = &data.currSlot;

	// retry previously-failed load
	if (sdata->queueRetry != 0)
	{
		sdata->queueRetry = 0;
	}

	// brand new load
	else
	{
		// Naughty Dog had inline copying,
		// is that faster on real PS1 hardware?

		memcpy(curr, &sdata->queueSlots[0], sizeof(struct LoadQueueSlot));

		for (int i = 1; i < sdata->queueLength; i++)
			memcpy(&sdata->queueSlots[i - 1], &sdata->queueSlots[i], sizeof(struct LoadQueueSlot));
	}

	sdata->queueLength--;

	u32 loadType = curr->flags;
	if (curr->type_UNUSED == LT_RAW)
		loadType = LT_SETADDR;
	else if (curr->type_UNUSED == LT_DRAM)
		loadType = LT_GETADDR;
	else if (curr->type_UNUSED == LT_VRAM)
		loadType = LT_SETVRAM;
	curr->flags = loadType;

	// get value originally passed
	// BEFORE calling ReadFile, which may change it
	void **prevValue = curr->ptrDestination;

	void *forceSetAddr = NULL;
	if ((loadType & LT_SETADDR) != 0)
		forceSetAddr = prevValue;

	void (*successCallback)(struct LoadQueueSlot *) = LOAD_CDRequestCallback;
	if ((loadType & LT_GETADDR) != 0)
		successCallback = LOAD_DramFileCallback;
	else if ((loadType & LT_SETVRAM) != 0)
		successCallback = LOAD_VramFileCallback;

	void *rawDestination =
	    LOAD_ReadFile_ex(curr->ptrBigfileCdPos_UNUSED, loadType | LT_ASYNC, curr->subfileIndex, forceSetAddr, &curr->size_UNUSED, successCallback);

	if (((loadType & LT_GETADDR) != 0) && (prevValue != NULL))
	{
		if (curr->callbackFuncPtr == (void (*)(struct LoadQueueSlot *))-2)
		{
			*prevValue = rawDestination;
		}
		else if (curr->ptrDestination == rawDestination)
		{
			*prevValue = (char *)rawDestination + 4;
		}
		else
		{
			// NOTE(aalhendi): Native PC reads can invoke the async callback before
			// ReadFile returns. In that case LOAD_DramFileCallback already skipped
			// the DRAM header.
			*prevValue = curr->ptrDestination;
		}
	}
}
