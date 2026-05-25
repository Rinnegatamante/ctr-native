#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80032d8c-0x80032dc0.
void LOAD_CDRequestCallback(struct LoadQueueSlot *lqs)
{
	if (lqs->callbackFuncPtr != NULL)
	{
		lqs->callbackFuncPtr(lqs);
	}

	sdata->queueReady = 1;
}
