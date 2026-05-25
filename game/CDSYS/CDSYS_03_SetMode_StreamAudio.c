#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8001c4f4-0x8001c56c.
void CDSYS_SetMode_StreamAudio()
{
	char buf[8];

	if (sdata->boolUseDisc == 0)
		return;

	if (sdata->bool_XnfLoaded == 0)
		return;

	if (sdata->discMode == DM_AUDIO)
		return;

	// https://www.cybdyn-systems.com.au/forum/viewtopic.php?t=1956
	// CdControl('\x0e',local_10,(u8 *)0x0);
	// param_1: 0xe = CdlSetmode
	// param_2: 0xE8 = set speed, play ADPCM, set sector
	// param_3: 0 = normal speed, 1 = double speed

	// Set Mode to Audio
	buf[0] = 0xE8;
	CdControl(CdlSetmode, buf, 0);

	sdata->discMode = DM_AUDIO;
	sdata->XA_State = 0;

	CdSyncCallback(CDSYS_XaCallbackCdSync);
	CdReadyCallback(CDSYS_XaCallbackCdReady);
}
