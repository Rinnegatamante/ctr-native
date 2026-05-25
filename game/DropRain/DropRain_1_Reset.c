#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002451c-0x80024524.
void DropRain_Reset(struct GameTracker *gGT)
{
	gGT->rainSoundID = 0;
	return;
}
