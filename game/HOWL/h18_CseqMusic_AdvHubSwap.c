#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800290cc-0x800291a0
void CseqMusic_AdvHubSwap(u16 songId, struct SongSet *songSet, int songSetActiveBits)
{
	struct Song *song;
	int i;

	if (sdata->boolAudioEnabled == 0)
		return;
	if (sdata->ptrCseqHeader == 0)
		return;
	if (sdata->ptrCseqHeader->numSongs <= songId)
		return;

	Smart_EnterCriticalSection();

	for (i = 0; i < 2; i++)
	{
		song = &sdata->songPool[i];

		// if song is playing
		if (song->flags & 1)
		{
			if (song->id == songId)
			{
				SongPool_AdvHub2(song, songSet, songSetActiveBits);
			}
		}
	}

	Smart_ExitCriticalSection();
	return;
}
