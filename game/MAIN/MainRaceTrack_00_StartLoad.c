#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003cf7c-0x8003cfc0
void DECOMP_MainRaceTrack_StartLoad(s16 levelID)

{
	// clear backup,
	// keep music,
	// destroy "most" fx, let menu fx play to end
	DECOMP_howl_StopAudio(1, 0, 0);

	DECOMP_ElimBG_Deactivate(sdata->gGT);

	DECOMP_LOAD_LevelFile(levelID);
	return;
}
