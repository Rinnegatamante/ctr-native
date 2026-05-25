#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002b540-0x8002b5b4
void Channel_SetVolume(struct ChannelAttr *attr, int volume, int LR)
{
	if ((u32)volume >= 0x4000)
		volume = 0x3fff;

	if (sdata->boolStereoEnabled == 1)
	{
		attr->audioL = (volume * data.volumeLR[0xFF - LR]) >> 8;
		attr->audioR = (volume * data.volumeLR[0x00 + LR]) >> 8;
		return;
	}

	attr->audioL = volume;
	attr->audioR = volume;
	return;
}
