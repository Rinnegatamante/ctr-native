#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002acb8-0x8002ad04
void UpdateChannelVol_EngineFX(struct EngineFX *engineFX, struct ChannelAttr *attr, int vol, int LR)
{
	Channel_SetVolume(attr, (sdata->vol_FX * engineFX->volume * vol) >> 10, LR);
}
