#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002ae64-0x8002af6c
void UpdateChannelVol_EngineFX_All()
{
	struct ChannelStats *curr;
	u32 *flagPtr;

	for (curr = (struct ChannelStats *)sdata->channelTaken.first; curr != NULL; curr = curr->next)
	{
		// type == MUSIC, skip
		if (curr->type == 2)
			continue;

		// update volume
		sdata->ChannelUpdateFlags[curr->channelID] |= 0x40;

		// just the sound, not the instance of sound
		int soundID = curr->soundID & 0xffff;

		// type == EngineFX
		if (curr->type == 0)
		{
			UpdateChannelVol_EngineFX(&sdata->howl_metaEngineFX[soundID], &sdata->channelAttrNew[curr->channelID], curr->vol, curr->LR);
		}

		// type == OtherFX
		else
		{
			UpdateChannelVol_OtherFX(&sdata->howl_metaOtherFX[soundID], &sdata->channelAttrNew[curr->channelID], curr->vol, curr->LR);
		}
	}
}
