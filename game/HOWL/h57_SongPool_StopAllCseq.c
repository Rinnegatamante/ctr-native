#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002ac0c-0x8002ac94
void SongPool_StopAllCseq(struct Song *song)
{
	int i;

	// if song is not playing, skip
	if ((song->flags & 1) == 0)
		return;

	for (i = 0; i < song->numSequences; i++)
	{
		SongPool_StopCseq(song->CseqSequences[i]);
	}

	// stop song
	song->flags &= (u8)~1;
}
