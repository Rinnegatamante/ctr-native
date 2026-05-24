#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80031c78-0x80031d30
void *DECOMP_LOAD_ReadDirectory(char *filename)
{
	CdlFILE cdlFile;
	char buf[8];

	DECOMP_CDSYS_SetMode_StreamData();

	if (CdSearchFile(&cdlFile, filename) == NULL)
		return NULL;

	struct BigHeader *bh = DECOMP_MEMPACK_AllocMem(0x4000 /*, filename*/);

	// Search for file on disc
	// Set Cd laser to file position
	// Read the bigfile header
	// Wait for read to end
	CdControl(CdlSetloc, &cdlFile, buf);
	if (CdRead(8, (u32 *)bh, 0x80) == 0)
		return NULL;

	if (CdReadSync(0, 0) != 0)
		return NULL;

	// Save position
	bh->cdpos = CdPosToInt(&cdlFile.pos);

	// undo allocation of 0x4000, only use "needed" size
	DECOMP_MEMPACK_ReallocMem(sizeof(struct BigHeader) + sizeof(struct BigEntry) * bh->numEntry);

	sdata->ptrBigfileCdPos_2 = bh;
	return bh;
}
