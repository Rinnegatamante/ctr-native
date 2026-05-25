#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002af6c-0x8002b030
void UpdateChannelVol_Music_All()
{
	struct ChannelStats *curr, *backupNext;

	for (curr = (struct ChannelStats *)sdata->channelTaken.first; curr != NULL; curr = backupNext)
	{
		backupNext = curr->next;

		// type != MUSIC, skip
		if (curr->type != 2)
			continue;

		// update volume
		sdata->ChannelUpdateFlags[curr->channelID] |= 0x40;

		UpdateChannelVol_Music(&sdata->songSeq[curr->soundID & 0xffff], &sdata->channelAttrNew[curr->channelID], curr->drumIndex_pitchIndex, curr->vol);
	}
}
