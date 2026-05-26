#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80032110-0x800321b4.
void LOAD_ReadFileASyncCallback(CdlIntrResult result, u8 *unk)
{
	CdReadCallback(0);
	result &= 0xff;

	struct LoadQueueSlot *lqs = &data.currSlot;

	if (result == CdlComplete)
	{
#if defined(CTR_NATIVE)
		if ((lqs->flags & LT_MEMPACK) != 0)
#else
		if ((lqs->flags & LT_SETADDR) != 0)
#endif
		{
			MEMPACK_ReallocMem(lqs->size_UNUSED);
		}

		if (sdata->callbackCdReadSuccess != NULL)
		{
			sdata->callbackCdReadSuccess(lqs);
		}
	}

	// CdlDiskError
	else
	{
#if defined(CTR_NATIVE)
		if ((lqs->flags & LT_MEMPACK) != 0)
#else
		if ((lqs->flags & LT_SETADDR) != 0)
#endif
		{
			// undo allocation, try again
			MEMPACK_ReallocMem(0);
		}

		sdata->queueRetry = 1;
	}
}
