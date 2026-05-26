#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80025138-0x800251ac.
void FLARE_Init(s16 *pos)
{
	// 0xc = size
	// 0 = no relation to param4
	// 0x300 = SmallStackPool
	// 0xd = "other" thread bucket
	struct Thread *th = PROC_BirthWithObject(0xc030d, FLARE_ThTick, data.s_lensflare, NULL);
	if (th != NULL)
	{
		// Get the pointer to flare, attached to the thread
		int *flare = th->object;
		*flare = 0; // frameCount = 0
		memcpy(&flare[1], pos, 2 * sizeof(int));
	}
}
