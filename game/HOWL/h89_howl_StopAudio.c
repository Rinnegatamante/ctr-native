#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002c8a8-0x8002c918
void DECOMP_howl_StopAudio(int boolErasePauseBackup, int boolEraseMusic, int boolDestroyAllFX)
{
	if (boolEraseMusic != 0)
		CseqMusic_StopAll();

	Smart_EnterCriticalSection();
	Channel_DestroyAll_LowLevel(boolDestroyAllFX, boolEraseMusic == 0, 2);
	Smart_ExitCriticalSection();

	if (boolErasePauseBackup != 0)
		sdata->numBackup_ChannelStats = 0;
}
