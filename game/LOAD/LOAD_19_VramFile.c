#include <common.h>

// NOTE(aalhendi): ASM-audited NTSC-U 926 0x80031fdc-0x80032110; this preserves
// the retail -1 startup VRAM slot path over native LOAD_ReadFile.
void *DECOMP_LOAD_VramFile(void *bigfilePtr, int subfileIndex, void *ptrDestination, int *sizePtr, int callbackOrFlags)
{
	struct LoadQueueSlot lqs;
	void *loadedFile;

	if (ptrDestination == NULL)
		MEMPACK_PushState();

	if (sizePtr != NULL)
		*sizePtr = 0;

	if (callbackOrFlags == -2)
	{
		loadedFile = DECOMP_LOAD_ReadFile(bigfilePtr, LT_SETVRAM | LT_SYNC, subfileIndex, NULL);
		if (ptrDestination != NULL)
			*(void **)ptrDestination = loadedFile;
		return loadedFile;
	}

	loadedFile = DECOMP_LOAD_ReadFile(bigfilePtr, LT_SETVRAM | LT_SYNC, subfileIndex, ptrDestination);

	if (callbackOrFlags == -1)
	{
		lqs.ptrBigfileCdPos_UNUSED = bigfilePtr;
		lqs.flags = 0;
		lqs.type_UNUSED = LT_VRAM;
		lqs.subfileIndex = subfileIndex;
		lqs.ptrDestination = loadedFile;
		lqs.size_UNUSED = sizePtr != NULL ? *sizePtr : 0;
		lqs.callbackFuncPtr = NULL;

		DECOMP_LOAD_VramFileCallback(&lqs);

		VSync(2);
		sdata->frameFinishedVRAM = 0;

		if (ptrDestination == NULL)
			MEMPACK_PopState();
	}

	return loadedFile;
}
