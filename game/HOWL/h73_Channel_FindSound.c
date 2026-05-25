#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002b5b4-0x8002b608
int Channel_FindSound(int soundID)
{
	struct ChannelStats *curr, *backupNext;

	for (curr = (struct ChannelStats *)sdata->channelTaken.first; curr != NULL; curr = backupNext)
	{
		backupNext = curr->next;

		if (
		    // type == OtherFX
		    (curr->type == 1) &&

		    // matching low 16-bit sound ID
		    ((curr->soundID & 0xffff) == (soundID & 0xffff)))
		{
			// sound already playing
			return 1;
		}
	}

	// sound not playing
	return 0;
}
