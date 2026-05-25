#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8001cc18-0x8001cd20.
int CDSYS_XASeek(int boolCdControl, int categoryID, int xaID)
{
	CdlLOC loc;
	int com;

	if (sdata->boolUseDisc == 0)
		return 1;

	if (sdata->bool_XnfLoaded == 0)
		return 0;

	if (categoryID >= CDSYS_XA_NUM_TYPES)
		return 0;

	if (xaID >= CDSYS_XAGetNumTracks(categoryID))
		return 0;

	if (sdata->discMode != DM_AUDIO)
		CDSYS_SetMode_StreamAudio();

	struct XaSize *xas = &sdata->ptrArray_XaSize[sdata->ptrArray_firstSongIndex[categoryID] + xaID];
	int sum = sdata->ptrArray_XaCdPos[sdata->ptrArray_firstXaIndex[categoryID] + xas->XaPrefix];

	CdIntToPos(sum, &loc);

	sdata->XA_State = 1;

	com = CdlSeekP;
	if (boolCdControl != 0)
		com = CdlSeekL;

	CdControl(com, &loc, 0);

	return 1;
}
