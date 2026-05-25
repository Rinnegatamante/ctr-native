#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002cbb4-0x8002cbe8
void Voiceline_ToggleEnable(int toggle)
{
	// if this is disabling
	if (toggle == 0)
	{
		sdata->voicelineCooldown = 0;

		Voiceline_StopAll();
	}
	sdata->boolCanPlayVoicelines = toggle;
}
