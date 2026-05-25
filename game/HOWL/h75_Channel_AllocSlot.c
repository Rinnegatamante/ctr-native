#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002b7d0-0x8002b898
struct ChannelStats *Channel_AllocSlot(int flags, struct ChannelAttr *attr)
{
	int i;
	struct ChannelAttr *newAttr;
	struct ChannelStats *stats;

	// get free slot
	stats = (struct ChannelStats *)sdata->channelFree.first;

	// quit if no free slots
	if (stats == NULL)
		return NULL;

	// allocate
	LIST_RemoveMember(&sdata->channelFree, (struct Item *)stats);
	LIST_AddBack(&sdata->channelTaken, (struct Item *)stats);

	// start playing
	sdata->ChannelUpdateFlags[stats->channelID] |= (flags | 2);

	// make new ChanenlAttr
	newAttr = &sdata->channelAttrNew[stats->channelID];

	// copy all 0x10 bytes in ChannelAttr
	for (i = 0; i < 4; i++)
	{
		((int *)newAttr)[i] = ((int *)attr)[i];
	}

	stats->flags = 1;

	return stats;
}
