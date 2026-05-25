#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002b030-0x8002b0e0
void UpdateChannelVol_OtherFX_All()
{
	struct ChannelStats *curr, *backupNext;

	for (curr = (struct ChannelStats *)sdata->channelTaken.first; curr != NULL; curr = backupNext)
	{
		backupNext = curr->next;

		// type != OtherFX, skip
		if (curr->type != 1)
			continue;

		// update volume
		sdata->ChannelUpdateFlags[curr->channelID] |= 0x40;

		UpdateChannelVol_OtherFX(&sdata->howl_metaOtherFX[curr->soundID & 0xffff], &sdata->channelAttrNew[curr->channelID], curr->vol, curr->LR);
	}
}
