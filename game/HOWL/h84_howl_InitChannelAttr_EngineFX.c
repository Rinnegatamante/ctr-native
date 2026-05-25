#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002c34c-0x8002c424
void DECOMP_howl_InitChannelAttr_EngineFX(struct EngineFX *engineFX, struct ChannelAttr *attr, int vol, int LR, int distort)
{
	Channel_SetVolume(attr, (sdata->vol_FX * engineFX->volume * vol) >> 10, LR);

	s16 pitch = engineFX->pitch;

	if (distort != 0x80)
		pitch = ((u32)pitch * data.distortConst_Engine[distort]) >> 0x10;

	attr->pitch = pitch;

	// ADSR
	attr->ad = 0x80ff;
	attr->sr = 0x1fc2;

	attr->spuStartAddr = (void *)(sdata->howl_spuAddrs[engineFX->spuIndex].spuAddr << 3);
}
