#include <common.h>

// Used for XNF and only the XNF
// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80032344-0x80032438
void *DECOMP_LOAD_XnfFile(char *filename, void *ptrDestination, int *size)
{
	CdlFILE cdlFile;

	DECOMP_LOAD_StringToUpper(filename);
	CDSYS_SetMode_StreamData();

	if (CdSearchFile(&cdlFile, filename) == 0)
		return 0;

	*size = cdlFile.size;

	if (ptrDestination == 0)
	{
		// allocate room for all sectors,
		// remove alignment before next Read
		int sectorSize = (cdlFile.size + 0x7ffU) & 0xfffff800;
		ptrDestination = MEMPACK_AllocMem(sectorSize /*, fileName*/);
		if (ptrDestination == NULL)
			return NULL;

		MEMPACK_ReallocMem(cdlFile.size);
	}

	char buf[8];
	CdControl(CdlSetloc, &cdlFile, buf);

	if (CdRead((cdlFile.size + 0x7ff) >> 0xb, ptrDestination, 0x80) == 0)
		return 0;

	if (CdReadSync(0, 0))
		return 0;

	return ptrDestination;
}
