#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002c1d0-0x8002c208
void Cutscene_VolumeRestore(void)
{
	// enter critical section
	Smart_EnterCriticalSection();

	// copy does not exist
	sdata->boolStoringVolume = 0;

	// Set volume of FX
	DECOMP_howl_VolumeSet(0, sdata->storedVolume);

	// exit critical section
	Smart_ExitCriticalSection();

	return;
}
