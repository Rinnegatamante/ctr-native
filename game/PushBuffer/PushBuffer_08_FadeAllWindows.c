#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80043ab8-0x80043b30.
void PushBuffer_FadeAllWindows()
{
	struct GameTracker *gGT = sdata->gGT;

	for (int i = 0; i < gGT->numPlyrCurrGame; i++)
	{
		PushBuffer_FadeOneWindow(&gGT->pushBuffer[i]);
	}

	PushBuffer_FadeOneWindow(&gGT->pushBuffer_UI);
}
