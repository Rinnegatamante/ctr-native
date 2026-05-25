#include <common.h>

// specific instance of soundID
// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80028808-0x80028844
void OtherFX_Stop1(int soundID_count)
{
	Smart_EnterCriticalSection();

	// specific instance of soundID
	Channel_SearchFX_Destroy(1, soundID_count, 0xffffffff);

	Smart_ExitCriticalSection();
}

void DECOMP_OtherFX_Stop1(int soundID_count)
{
	OtherFX_Stop1(soundID_count);
}
