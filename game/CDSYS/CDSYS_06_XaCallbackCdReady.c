#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8001c7fc-0x8001c8e4.
void CDSYS_XaCallbackCdReady(CdlIntrResult result, u8 *unk) //+unk to adhere to *CdlCB
{
	if (result == CdlDataReady)
	{
		CdGetSector(&sdata->cdlFile_CdReady[0], 3);
		sdata->XA_CurrPos = CdPosToInt(&sdata->cdlFile_CdReady[0]);

		if (
		    // queued to start (CD seeking?)
		    (sdata->XA_State == 2) && (sdata->XA_StartPos <= sdata->XA_CurrPos))
		{
			// XA playing
			sdata->XA_State = 3;

			sdata->XA_CurrOffset = (sdata->XA_CurrPos - sdata->XA_StartPos) * 4;
		}

		if (
		    // XA is playing
		    (sdata->XA_State == 3) && (sdata->XA_EndPos < sdata->XA_CurrPos))
		{
			// XA should stop
			sdata->XA_State = 4;

			// disable music
			sdata->XA_VolumeDeduct = 0x400;
		}

		sdata->countPass_CdReadyCallback++;
		return;
	}

	sdata->countFail_CdReadyCallback++;
}
