#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800afa94-0x800afaf0.
void MM_TrackSelect_Video_State(int state)
{
	// if viewing new icon this frame
	if (state == 1)
	{
		// icon has been viewed for zero frames
		D230.trackSel_video_frameCount = 0;

		// player sees a track icon (not video)
		D230.trackSel_videoStateCurr = 1;

		return;
	}

#ifdef CTR_NATIVE
	// NOTE(aalhendi): Native does not compile the STR/MDEC preview path yet.
	return;
#endif

	// if player sees a track icon
	if (D230.trackSel_videoStateCurr == 1)
	{
		// wait 20 frames
		if (D230.trackSel_video_frameCount < 21)
		{
			D230.trackSel_video_frameCount++;
		}
		else
		{
			// allocate video memory, prepare to play video
			D230.trackSel_videoStateCurr = 2;
		}
	}
}
