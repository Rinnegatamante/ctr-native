#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80029b2c-0x80029c40
int DECOMP_howl_LoadHeader(char *filename)
{
	struct HowlHeader *alloc;
	int howlHeaderSize;
	int numSector;
	int ret;

	if (DECOMP_LOAD_FindFile(filename, &sdata->KartHWL_CdFile) == 0)
		return 0;

	MEMPACK_PushState();

	// allocate room for one sector
	alloc = MEMPACK_AllocMem(0x800 /*, filename*/);

	if (alloc != 0)
	{
		// read sector #1 of HOWL, just for header
		ret = DECOMP_LOAD_HowlHeaderSectors(&sdata->KartHWL_CdFile, alloc, 0, 1);

		if (
		    // confirm first sector loaded properly
		    (ret != 0) && (alloc->magic == *(int *)&sdata->s_HOWL[0]) && (alloc->version == 0x80) // different in other CTR builds
		)
		{
			// allocate room for howlHeader + pointerTable
			howlHeaderSize = sizeof(struct HowlHeader) + alloc->headerSize;

			// align up for sector size
			numSector = (howlHeaderSize + 0x800 - 1) >> 0xb;
			MEMPACK_ReallocMem(numSector << 0xb);

			// if header needs more sectors loaded, like CTR-U which needs 3 sectors
			if (numSector < 2 || DECOMP_LOAD_HowlHeaderSectors(&sdata->KartHWL_CdFile, (void *)((int)alloc + 0x800), 1, numSector - 1) != 0)
			{
				// initilaize header and pointer table
				DECOMP_howl_ParseHeader(alloc);

				// reallocate room just howlHeader + pointerTable,
				// deallocate sector-alignment padding
				MEMPACK_ReallocMem(howlHeaderSize);

				// do NOT PopState
				return 1;
			}
		}
	}

	MEMPACK_PopState();
	return 0;
}
