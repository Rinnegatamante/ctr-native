#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002cae0-0x8002cb44
void Voiceline_PoolClear(void)
{
	sdata->boolCanPlayWrongWaySFX = false;

	sdata->voicelineCooldown = 0;

	sdata->boolCanPlayVoicelines = false;

	LIST_Clear(&sdata->Voiceline1);

	LIST_Clear(&sdata->Voiceline2);

	// put them all on free list
	LIST_Init(&sdata->Voiceline1, (struct Item *)&sdata->voicelinePool[0].next, 0x10, 8);

	Voiceline_ClearTimeStamp();
}
