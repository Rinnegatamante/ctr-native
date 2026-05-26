#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80046a90-0x80046b1c.
int RefreshCard_GetResult(int result)
{
	s16 result16 = result;

	if (result16 == 8)
	{
		if ((sdata->memcardUnk1 & 6) != 0)
			return 1;
	}

	if ((sdata->memcardUnk1 & 6) != 0)
		return 0;

	if (sdata->frame3_memcardAction != sdata->frame4_memcardAction)
		return 0;

	if (sdata->frame3_memcardSlot != sdata->frame4_memcardSlot)
		return 0;

	return sdata->desired_memcardResult == result16;
}
