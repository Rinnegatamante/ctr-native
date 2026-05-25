#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8001c360-0x8001c420.
int CDSYS_Init(int boolUseDisc)
{
	sdata->boolUseDisc = boolUseDisc;

	*(s16 *)&sdata->unused400[0] = 0;

	// if using parallel port (Naughty Dog Devs only)
	if (boolUseDisc != 0)
	{
		// if Cd does not initialize
		if (CdInit() == 0)
		{
			// use parallel port (Naughty Dog Devs only)
			sdata->boolUseDisc = 0;
			return 0;
		}

		CdSetDebug(1);
	}

	sdata->discMode = -1;
	sdata->bool_XnfLoaded = 0;

	sdata->XA_State = 0;
	sdata->XA_boolFinished = 0;

	sdata->countFail_CdSyncCallback = 0;
	sdata->countPass_CdReadyCallback = 0;
	sdata->countFail_CdReadyCallback = 0;
	sdata->XA_CurrOffset = 0;
	sdata->countPass_CdTransferCallback = 0;

	sdata->XA_Playing_Category = 0;
	sdata->XA_Playing_Index = 0;

	sdata->XA_StartPos = 0;
	sdata->XA_EndPos = 0;

	sdata->XA_VolumeBitshift = 0;
	sdata->XA_VolumeDeduct = 0;

	sdata->XA_MaxSampleVal = 0;
	sdata->XA_MaxSampleValInArr = 0;

	sdata->irqAddr = 0;

	// unused_8008d700

	sdata->XA_CurrPos = 0;
	sdata->XA_PauseFrame = 0;

	CDSYS_SetMode_StreamData();

	// 1 - English
	CDSYS_SetXAToLang(1);

	Voiceline_PoolClear();

	return 1;
}
