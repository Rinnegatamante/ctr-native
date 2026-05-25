#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80028b54-0x80028bbc
void EngineAudio_Stop(u32 soundID)
{
	if (sdata->boolAudioEnabled == 0)
		return;

	soundID = soundID & 0xffff;
	if (sdata->ptrHowlHeader->numEngineFX <= (int)soundID)
		return;

	// 0 - engineFX
	Smart_EnterCriticalSection();
	Channel_SearchFX_Destroy(0, soundID, 0xffffffff);
	Smart_ExitCriticalSection();

	return;
}
