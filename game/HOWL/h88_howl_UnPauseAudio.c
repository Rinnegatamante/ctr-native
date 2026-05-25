#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002c784-0x8002c8a8
void DECOMP_howl_UnPauseAudio()
{
	int i;
	char backupID;
	struct ChannelStats *backupPrev;
	struct ChannelStats *curr, *backupNext;
	struct ChannelStats *pausedStats;

	// if no paused audio, skip
	if (sdata->numBackup_ChannelStats == 0)
		return;

	pausedStats = &sdata->channelStatsCurr[0];

	DECOMP_Smart_EnterCriticalSection();
	for (i = 0, curr = (struct ChannelStats *)sdata->channelFree.first; i < sdata->numBackup_ChannelStats; i++, curr = backupNext)
	{
		backupID = curr->channelID;
		backupPrev = curr->prev;
		backupNext = curr->next;

		int *src = (int *)pausedStats++;
		int *dest = (int *)curr;

		// psx's kernel memcpy does NOT work inside "critical" sections
		dest[0] = src[0];
		dest[1] = src[1];
		dest[2] = src[2];
		dest[3] = src[3];
		dest[4] = src[4];
		dest[5] = src[5];
		dest[6] = src[6];
		dest[7] = src[7];

		curr->next = backupNext;
		curr->prev = backupPrev;
		curr->channelID = backupID;

		LIST_RemoveMember(&sdata->channelFree, (struct Item *)curr);
		LIST_AddBack(&sdata->channelTaken, (struct Item *)curr);

		DECOMP_howl_UnPauseChannel(curr);
	}
	DECOMP_Smart_ExitCriticalSection();

	DECOMP_CseqMusic_Resume();

	sdata->numBackup_ChannelStats = 0;
}
