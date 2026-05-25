#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002aa44-0x8002ab18
void SongPool_AdvHub2(struct Song *song, struct SongSet *songSet, int songSetActiveBits)
{
	int i;
	int vol;
	struct CseqSongHeader *csh = (struct CseqSongHeader *)&sdata->ptrCseqSongData[sdata->ptrCseqSongStartOffset[song->id]];
	u8 numSeqs = csh->numSeqs;

	// advHub
	if (songSet != 0)
	{
		if (songSet->numSeqs != numSeqs)
			return;

		song->songSetActiveBits = songSetActiveBits;
	}

	for (i = 0; i < numSeqs; i++)
	{
		// volume on
		vol = 0xff;

		if ((songSet->ptrSongSetBits[i] & song->songSetActiveBits) == 0)
			vol = 0;

		SongPool_AdvHub1(song, i, vol, 0);
	}
}
