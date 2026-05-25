#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8001cbe0-0x8001cc18.
int CDSYS_XAGetNumTracks(int categoryID)
{
	if (sdata->boolUseDisc == 0)
		return 0;
	if (categoryID >= CDSYS_XA_NUM_TYPES)
		return 0;

	return sdata->ptrArray_numSongs[categoryID];
}
