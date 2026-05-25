#include <common.h>

// all instances of soundID
// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80028844-0x80028880
void OtherFX_Stop2(int soundID_count)
{
	Smart_EnterCriticalSection();

	// all instances of soundID
	Channel_SearchFX_Destroy(1, soundID_count & 0xffff, 0xffff);

	Smart_ExitCriticalSection();
}

void DECOMP_OtherFX_Stop2(int soundID_count)
{
	OtherFX_Stop2(soundID_count);
}
