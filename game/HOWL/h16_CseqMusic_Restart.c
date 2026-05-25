#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80028f34-0x80029008
void CseqMusic_Restart(u16 songID, int p2)
{
	int i;
	struct Song *song;

	if (sdata->boolAudioEnabled == 0)
		return;
	if (sdata->ptrCseqHeader == 0)
		return;
	if (sdata->ptrCseqHeader->numSongs <= songID)
		return;

	Smart_EnterCriticalSection();

	for (i = 0; i < 2; i++)
	{
		song = &sdata->songPool[i];

		// if pool is taken
		if (((song->flags & 1) != 0) && (song->id == songID))
		{
			song->flags |= 4;
			SongPool_Volume(song, 0, p2 & 0xff, 0);
		}
	}

	Smart_ExitCriticalSection();
}
