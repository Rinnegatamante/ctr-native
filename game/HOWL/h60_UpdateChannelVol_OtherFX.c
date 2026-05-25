#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002ad04-0x8002ad70
void UpdateChannelVol_OtherFX(struct OtherFX *otherFX, struct ChannelAttr *attr, int vol, int LR)
{
	int otherVol;

	otherVol = sdata->vol_FX;

	if ((otherFX->flags & 4) != 0)
		otherVol = sdata->vol_Voice;

	Channel_SetVolume(attr, (otherVol * otherFX->volume * vol) >> 10, LR);
}
