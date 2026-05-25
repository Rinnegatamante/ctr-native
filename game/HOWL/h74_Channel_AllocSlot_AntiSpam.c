#include <common.h>

void Channel_DestroySelf(struct ChannelStats *stats);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002b608-0x8002b7d0
struct ChannelStats *Channel_AllocSlot_AntiSpam(s16 soundID, char boolUseAntiSpam, int flags, struct ChannelAttr *attr)
{
	struct ChannelAttr *newAttr;
	struct ChannelStats *curr, *backupNext;

	// with AntiSpam, a new sound started within
	// 10 frames of another, will replace the older

	// without AntiSpam, sounds within 10 frames
	// of each other will play until they all finish

	if (boolUseAntiSpam == 1)
	{
		for (curr = (struct ChannelStats *)sdata->channelTaken.first; curr != NULL; curr = backupNext)
		{
			backupNext = curr->next;

			if (
			    // type == OtherFX
			    (curr->type == 1) &&

			    // matching ID
			    ((curr->soundID & 0xffff) == ((u16)soundID)))
			{
				int duration = sdata->gGT->frameTimer_MainFrame_ResetDB - curr->startFrame;

				// if started within 10 frames, cancel old and start new,
				// otherwise you'll allocate too many sounds and overflow
				if (duration < 10)
				{
					Channel_DestroySelf(curr);
				}
			}
		}
	}

	return Channel_AllocSlot(flags, attr);
}

void Channel_DestroySelf(struct ChannelStats *stats)
{
	// set channel to OFF, and remove PLAYING bit
	u32 *flagPtr = &sdata->ChannelUpdateFlags[stats->channelID];
	*flagPtr |= 1;
	*flagPtr &= ~(2);

	stats->flags &= ~(1);

	// recycle
	LIST_RemoveMember(&sdata->channelTaken, (struct Item *)stats);
	LIST_AddBack(&sdata->channelFree, (struct Item *)stats);
}
