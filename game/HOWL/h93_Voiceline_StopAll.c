#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002cb44-0x8002cbb4
void DECOMP_Voiceline_StopAll(void)
{
	while (sdata->Voiceline2.last != 0)
	{
		struct Item *voiceLine = sdata->Voiceline2.last;

		LIST_RemoveMember(&sdata->Voiceline2, voiceLine);
		LIST_AddFront(&sdata->Voiceline1, voiceLine);
	}
}
