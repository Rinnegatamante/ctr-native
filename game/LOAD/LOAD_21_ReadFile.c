#include <common.h>

void *LOAD_ReadFile_ex(struct BigHeader *bigfile, u32 loadType, int subfileIndex, void *ptrDst, int *sizePtr, void (*callback)(struct LoadQueueSlot *))
{
	int uVar5;
	CdlLOC cdLoc;
	u8 paramOutput[8];
	void *originalDst;
	int sectorSize;
	int sectorCount;
	int readComplete;

	// NOTE(aalhendi): ASM-verified NTSC-U 926 PS1 path 0x800321b4-0x80032344.
	(void)loadType;
	CDSYS_SetMode_StreamData();

#if defined(CTR_NATIVE)
	// NOTE(aalhendi): CTR_NATIVE preserves existing queues that pass 0 for the
	// default bigfile; retail callers are expected to pass the real pointer.
	if (bigfile == NULL)
		bigfile = sdata->ptrBigfile1;
#endif

	// get size and offset of subfile
	struct BigEntry *entry = BIG_GETENTRY(bigfile);
	int eSize = entry[subfileIndex].size;
	int eOffs = entry[subfileIndex].offset;

	*sizePtr = eSize;

	CdIntToPos(bigfile->cdpos + eOffs, &cdLoc);

	struct LoadQueueSlot *lqs = &data.currSlot;
	originalDst = ptrDst;
	sectorCount = (eSize + 0x7ffU) >> 0xb;
	readComplete = 1;

	// If no address given, then find one.
	if (ptrDst == NULL)
	{
#if defined(CTR_NATIVE)
		lqs->flags |= LT_MEMPACK;
#else
		lqs->flags |= LT_SETADDR;
#endif

		// allocate room for all sectors,
		// remove alignment before next Read
		sectorSize = sectorCount << 0xb;
		ptrDst = (void *)MEMPACK_AllocMem(sectorSize); // "FILE"
		if (ptrDst == NULL)
			return NULL;
	}
	else
	{
#if defined(CTR_NATIVE)
		lqs->flags &= ~LT_MEMPACK;
#else
		lqs->flags &= ~LT_SETADDR;
#endif
	}

#if defined(CTR_NATIVE)
	// NOTE(aalhendi): native CD reads can call back before wrapper callers store
	// the returned pointer back into data.currSlot.
	lqs->ptrDestination = ptrDst;
	lqs->size_UNUSED = eSize;
#endif

	while (1)
	{
		uVar5 = CdControl(CdlSetloc, &cdLoc, &paramOutput[0]);

		if (callback != NULL)
		{
			sdata->callbackCdReadSuccess = callback;
			CdReadCallback(LOAD_ReadFileASyncCallback);
		}
		else
		{
			sdata->callbackCdReadSuccess = NULL;
			CdReadCallback(NULL);
		}

		uVar5 &= CdRead(sectorCount, ptrDst, 0x80);

		if (callback == NULL)
		{
			// Wait for all sectors to finish
			readComplete = CdReadSync(0, (u8 *)0x0) < 1;
		}

		// If either command failed, or sync read did not finish, retry.
		if ((uVar5 != 0) && (readComplete != 0))
			break;
	}

	if ((callback == NULL) && (originalDst == NULL))
	{
		MEMPACK_ReallocMem(*sizePtr);
	}

	return ptrDst;
}
