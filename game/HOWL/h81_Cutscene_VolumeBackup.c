#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002c18c-0x8002c1d0
void Cutscene_VolumeBackup(void)
{
	// enter critical section
	Smart_EnterCriticalSection();

	// make another copy volume of FX and clamp to 0x100
	sdata->storedVolume = DECOMP_howl_VolumeGet(0) & 0xff;
	sdata->currentVolume = sdata->storedVolume;

	// copy exists
	sdata->boolStoringVolume = 1;

	// exit critical section
	Smart_ExitCriticalSection();

	return;
}
