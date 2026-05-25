#include <common.h>

// "end of song" opcode
// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80029f24-0x80029f78
void cseq_opcode03(struct SongSeq *seq)
{
	// if song does not loop
	if ((seq->flags & 2) == 0)
	{
		SongPool_StopAllCseq(&sdata->songPool[seq->songPoolIndex]);
	}

	// if song loops
	else
	{
		// start over
		seq->flags |= 8;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80029f78-0x80029f80
void cseq_opcode04_empty(struct SongSeq *seq)
{
	// left empty by ND
}
