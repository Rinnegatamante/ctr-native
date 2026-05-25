#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800291a0-0x80029258
void CseqMusic_Stop(u16 songID)
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
			SongPool_StopAllCseq(song);
		}
	}

	Smart_ExitCriticalSection();
}
