#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002c510-0x8002c64c
void DECOMP_howl_PauseAudio()
{
	u32 *ptrFlag;
	struct ChannelStats *curr, *backupNext;
	struct ChannelStats *pausedStats;

	DECOMP_CDSYS_XAPauseRequest();

	// if already paused, quit
	if (sdata->numBackup_ChannelStats != 0)
		return;

	pausedStats = &sdata->channelStatsCurr[0];

	DECOMP_CseqMusic_Pause();

	DECOMP_Smart_EnterCriticalSection();
	for (curr = (struct ChannelStats *)sdata->channelTaken.first; curr != NULL; curr = backupNext)
	{
		backupNext = curr->next;

		ptrFlag = &sdata->ChannelUpdateFlags[curr->channelID];
		*ptrFlag |= 1;
		*ptrFlag &= ~(2);

		int *dest = (int *)pausedStats++;
		int *src = (int *)curr;

		// psx's kernel memcpy does NOT work inside "critical" sections
		dest[0] = src[0];
		dest[1] = src[1];
		dest[2] = src[2];
		dest[3] = src[3];
		dest[4] = src[4];
		dest[5] = src[5];
		dest[6] = src[6];
		dest[7] = src[7];

		LIST_RemoveMember(&sdata->channelTaken, (struct Item *)curr);
		LIST_AddBack(&sdata->channelFree, (struct Item *)curr);

		sdata->numBackup_ChannelStats++;
	}
	DECOMP_Smart_ExitCriticalSection();
}
