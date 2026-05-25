#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002a9f0-0x8002aa44
void SongPool_AdvHub1(struct Song *song, int seqID, int vol, int boolImm)
{
	struct SongSeq *seq;

	struct CseqSongHeader *csh = (struct CseqSongHeader *)&sdata->ptrCseqSongData[sdata->ptrCseqSongStartOffset[song->id]];

	if (seqID >= (u8)csh->numSeqs)
		return;

	seq = song->CseqSequences[seqID];

	// if immediate change
	if (boolImm != 0)
	{
		seq->vol_Curr = vol;
	}

	seq->vol_New = vol;
}
