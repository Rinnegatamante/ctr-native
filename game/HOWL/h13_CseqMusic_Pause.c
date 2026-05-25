#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80028d64-0x80028de0
// pause all songs
void CseqMusic_Pause()
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
			// pause song
			song->flags |= 2;
		}
	}

	Smart_ExitCriticalSection();
}
