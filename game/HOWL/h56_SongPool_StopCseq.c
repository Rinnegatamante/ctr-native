#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002ab18-0x8002ac0c
void SongPool_StopCseq(struct SongSeq *seq)
{
	struct ChannelStats *curr, *backupNext;
	u32 *flagPtr;

	for (curr = (struct ChannelStats *)sdata->channelTaken.first; curr != NULL; curr = backupNext)
	{
		backupNext = curr->next;

		// type != MUSIC
		if (curr->type != 2)
			continue;

		if (curr->soundID != seq->soundID)
			continue;

		// enable OFF(1) flag, disable ON(2) flag
		flagPtr = &sdata->ChannelUpdateFlags[curr->channelID];
		*flagPtr |= 1;
		*flagPtr &= ~(2);

		curr->flags &= (u8)~1;

		// recycle: remove from taken, put on free
		LIST_RemoveMember(&sdata->channelTaken, (struct Item *)curr);
		LIST_AddBack(&sdata->channelFree, (struct Item *)curr);
	}

	// not playing
	seq->flags &= ~(1);
}
