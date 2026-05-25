#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80028de0-0x80028e5c
// resume all songs
void CseqMusic_Resume()
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
			// unpause song
			song->flags &= ~(2);
		}
	}

	Smart_ExitCriticalSection();
}
