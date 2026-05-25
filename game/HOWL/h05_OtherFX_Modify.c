#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80028690-0x80028808
u32 DECOMP_OtherFX_Modify(u32 soundId, u32 flags)
{
	struct ChannelStats *channel;
	struct ChannelAttr channelAttr;
	int modify;
	struct OtherFX *ptrOtherFX;
	u32 distort;
	u32 volume;
	u16 echo;
	u16 LR;

	if (sdata->boolAudioEnabled == 0)
		return 0;

	// quit if out of bounds
	if ((sdata->ptrHowlHeader->numOtherFX) <= (int)(soundId & 0xffff))
		return 0;

	// metaOtherFX
	ptrOtherFX = &sdata->howl_metaOtherFX[soundId & 0xffff];
	volume = flags >> 0x10 & 0xff;
	distort = flags >> 8 & 0xff;
	echo = flags >> 0x18;
	LR = flags & 0xff;

	// volume of FX
	modify = sdata->vol_FX;
	if ((ptrOtherFX->flags & 4) != 0)
	{
		// volume of Voice
		modify = sdata->vol_Voice;
	}

	// no distortion
	if (distort == 0x80)
	{
		channelAttr.pitch = ptrOtherFX->pitch;
	}

	// distortion
	else
	{
		channelAttr.pitch = ptrOtherFX->pitch * data.distortConst_OtherFX[distort] >> 0x10;
	}

	Channel_SetVolume(&channelAttr, modify * ptrOtherFX->volume * volume >> 10, LR);
	channelAttr.reverb = echo;

	Smart_EnterCriticalSection();

	// 1 - otherFX
	// soundID & 0xffffffff, search for specific instance
	channel = Channel_SearchFX_EditAttr(1, soundId, 0x70, &channelAttr);

	if (channel != 0)
	{
		channel->echo = echo;
		channel->vol = volume;
		channel->distort = distort;
		channel->LR = LR;
	}

	Smart_ExitCriticalSection();

	return 1;
}
