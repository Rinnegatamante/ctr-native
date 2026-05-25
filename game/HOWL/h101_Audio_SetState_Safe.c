#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002d4cc-0x8002d50c
void Audio_SetState_Safe(int state)
{
	// If this sound isn't already playing
	if (state != sdata->unkAudioState)
	{
		Voiceline_EmptyFunc();

		Audio_SetState(state);

		// set which sound is playing
		sdata->unkAudioState = state;
	}
}
