#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80032498-0x8003254c
int DECOMP_LOAD_HowlHeaderSectors(CdlFILE *cdlFileHWL, void *ptrDestination, int firstSector, int numSector)
{
	CdlLOC loc;

	CDSYS_SetMode_StreamData();

	// Return error, if reading out-of-bounds after the end of KART HWL
	int sizeOver = ((firstSector + numSector) * 0x800 - cdlFileHWL->size);
	if (sizeOver >= 0x800)
		return 0;

	CdIntToPos(CdPosToInt(&cdlFileHWL->pos) + firstSector, &loc);

	char buf[8];
	CdControl(CdlSetloc, &loc, buf);

	if (CdRead(numSector, ptrDestination, 0x80) == 0)
		return 0;

	if (CdReadSync(0, 0) != 0)
		return 0;

	return 1;
}
