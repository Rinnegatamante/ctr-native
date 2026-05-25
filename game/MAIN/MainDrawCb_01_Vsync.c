#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80034aa4-0x80034b48;
// REBUILD_PC keeps native input/audio/VRAM bridge work.
void MainDrawCb_Vsync()
{
	struct GameTracker *gGT;

	gGT = sdata->gGT;
	gGT->frameTimer_VsyncCallback++;
	if ((gGT->gameMode1 & PAUSE_ALL) == 0)
		gGT->frameTimer_Confetti++;

	sdata->vsyncTillFlip--;
	gGT->vSync_between_drawSync++;

	// 1 unit = 1/16th millisecond
	// 1 second = ~16,000 units
	// increment timer, and reset system clock
	sdata->rcntTotalUnits += GetRCnt(0xf2000001);
	ResetRCnt(0xf2000001);

#ifdef REBUILD_PC
	// NOTE(aalhendi): Retail calls HOWL unconditionally. Native skips only while
	// channel lists are mid-edit.
	if (sdata->criticalSectionCount == 0)
#endif
	{
		DECOMP_howl_PlayAudio_Update();
	}

#ifdef REBUILD_PC
	PsyX_UpdateInput();
#endif

	GAMEPAD_PollVsync(sdata->gGamepads);

#ifdef REBUILD_PC

	// wait two vsyncs for VRAM upload to finish
	if (sdata->frameFinishedVRAM != 0)
	{
		sdata->frameFinishedVRAM--;

		if (sdata->frameFinishedVRAM == 0)
		{
			sdata->queueReady = 1;
		}
	}

#endif

	return;
}
