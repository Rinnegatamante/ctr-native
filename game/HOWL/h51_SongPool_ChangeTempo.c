#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002a6cc-0x8002a730
void SongPool_ChangeTempo(struct Song *song, s16 deltaBPM)
{
	struct CseqSongHeader *csh = (struct CseqSongHeader *)&sdata->ptrCseqSongData[sdata->ptrCseqSongStartOffset[song->id]];

	song->bpm = csh->bpm + deltaBPM;

	song->tempo = SongPool_CalculateTempo(60, song->tpqn, song->bpm);
}
