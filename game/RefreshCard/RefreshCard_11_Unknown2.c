#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800471e8-0x80047224.
void RefreshCard_Unknown2(void)
{
	if (*(s16 *)&sdata->boolAdvProfilesChecked == 0)
	{
		GAMEPROG_InitFullMemcard((struct MemcardProfile *)sdata->ptrToMemcardBuffer1);
		*(s16 *)&sdata->boolAdvProfilesChecked = 1;
	}

	*(s16 *)&sdata->unk8008d95c = 1;
	*(s16 *)&sdata->unk_memcardRelated_8008d928[0] = 0;
}
