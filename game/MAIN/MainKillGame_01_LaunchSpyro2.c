#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003c480-0x8003c508 for the retail shutdown path.
void DECOMP_MainKillGame_LaunchSpyro2(void)
{
	DECOMP_CTR_ErrorScreen(0, 0, 0);

	DECOMP_Music_Stop();

	// clear backup, destroy music, destroy all fx
	DECOMP_howl_StopAudio(1, 1, 1);

	DECOMP_Bank_DestroyAll();

	DECOMP_howl_Disable();

	VSync(0x1e);

	DECOMP_MainKillGame_StopCTR();

#ifdef REBUILD_PC

	while (1)
	{
	}

#else

	_96_remove();
	_96_init();

	LoadExec(&rdata.s_PathTo_SpyroExe[0], 0x801fff00, 0);

#endif
}
