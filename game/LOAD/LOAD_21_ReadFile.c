#include <common.h>

void *LOAD_ReadFile_ex(struct BigHeader *bigfile, u32 loadType, int subfileIndex, void *ptrDst, int *sizePtr, void (*callback)(struct LoadQueueSlot *))
{
	int uVar5;
	CdlLOC cdLoc;
	u8 paramOutput[8];
	void *requestedDst;
	int sectorSize;
	int isAsync;

	// NOTE(aalhendi): Source-audited NTSC-U 926 0x800321b4-0x80032344;
	// CTR_NATIVE still accepts the old loadType shim used by current callers.
	CDSYS_SetMode_StreamData();

	if (bigfile == NULL)
		bigfile = sdata->ptrBigfile1;

	// get size and offset of subfile
	struct BigEntry *entry = BIG_GETENTRY(bigfile);
	int eSize = entry[subfileIndex].size;
	int eOffs = entry[subfileIndex].offset;

	if (sizePtr != NULL)
		*sizePtr = eSize;

	CdIntToPos(bigfile->cdpos + eOffs, &cdLoc);

	struct LoadQueueSlot *lqs = &data.currSlot;
	requestedDst = ptrDst;

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
		sectorSize = (eSize + 0x7ffU) & 0xfffff800;
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

	lqs->ptrDestination = ptrDst;
	lqs->size_UNUSED = eSize;

	if (((loadType & LT_ASYNC) != 0) && (callback == NULL))
	{
#if defined(CTR_NATIVE)
		if ((loadType & LT_GETADDR) != 0)
			callback = LOAD_DramFileCallback;
		else if ((loadType & LT_SETVRAM) != 0)
			callback = LOAD_VramFileCallback;
		else if ((loadType & LT_ASYNC) != 0)
			callback = LOAD_CDRequestCallback;
#endif
	}

	isAsync = (callback != NULL) || ((loadType & LT_ASYNC) != 0);

	while (1)
	{
		uVar5 = CdControl(CdlSetloc, &cdLoc, &paramOutput[0]);

		if (isAsync)
		{
			sdata->callbackCdReadSuccess = callback;
			CdReadCallback(LOAD_ReadFileASyncCallback);
		}
		else
		{
			sdata->callbackCdReadSuccess = NULL;
			CdReadCallback(NULL);
		}

		uVar5 &= CdRead(eSize + 0x7ffU >> 0xb, ptrDst, 0x80);

		// if either command failed,
		// retry Control and Read again
		if (uVar5 == 0)
			continue;

		// Async commands passed successfully,
		// only stay in the loop for Sync loads
		if (isAsync)
			break;

		// Wait for all sectors to finish
		uVar5 = CdReadSync(0, (u8 *)0x0);

		// if ZERO sectors remain,
		// then Sync commands passed, end loop
		if (uVar5 == 0)
			break;
	}

	if ((isAsync == 0) && (requestedDst == NULL))
	{
		MEMPACK_ReallocMem(eSize);
	}

	return ptrDst;
}
