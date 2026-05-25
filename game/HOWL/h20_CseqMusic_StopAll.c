#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80029258-0x800292e0
void CseqMusic_StopAll()
{
	int i;
	struct Song *song;

	if (sdata->boolAudioEnabled == 0)
		return;
	if (sdata->ptrCseqHeader == 0)
		return;

	Smart_EnterCriticalSection();

	for (i = 0; i < 2; i++)
	{
		song = &sdata->songPool[i];

		// if pool is taken
		if ((song->flags & 1) != 0)
		{
			SongPool_StopAllCseq(song);
		}
	}

	Smart_ExitCriticalSection();
}
