#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80028e5c-0x80028f34
void CseqMusic_ChangeVolume(u16 songID, int p2, int p3)
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
			SongPool_Volume(song, p2 & 0xff, p3 & 0xff, 0);
		}
	}

	Smart_ExitCriticalSection();
}
