#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002a63c-0x8002a678
struct SongSeq *SongPool_FindFreeChannel(void)
{
	struct SongSeq *seq;

	for (seq = &sdata->songSeq[0]; seq < &sdata->songSeq[NUM_SFX_CHANNELS]; seq++)
	{
		// if seq is not playing
		if ((seq->flags & 1) == 0)
			return seq;
	}

	return 0;
}
