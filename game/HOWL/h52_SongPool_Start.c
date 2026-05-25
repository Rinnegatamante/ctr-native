#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002a730-0x8002a9d8
void SongPool_Start(struct Song *song, u16 songID, s16 deltaBPM, int boolLoopAtEnd, struct SongSet *songSet, int songSetActiveBits)
{
	int i;
	int vol;
	u8 numSeqs;
	struct SongSeq *seqCurr;
	struct CseqSongHeader *csh;
	char *cnhFirst;
	struct SongNoteHeader *cnhCurr;

	// now playing
	song->flags = 1;
	song->id = songID;

	csh = (struct CseqSongHeader *)&sdata->ptrCseqSongData[sdata->ptrCseqSongStartOffset[songID]];
	numSeqs = csh->numSeqs;

	// advHub
	if (songSet != 0)
	{
		if (songSet->numSeqs != numSeqs)
			return;

		song->songSetActiveBits = songSetActiveBits;
	}

	song->tpqn = csh->tpqn;
	song->bpm = csh->bpm + deltaBPM;

	song->tempo = SongPool_CalculateTempo(60, song->tpqn, song->bpm);

	song->unk10 = 0;
	song->timeSpentPlaying = 0;

	if ((u16)(songID - 1) < 2)
	{
		// full volume
		vol = 0xff;
	}

	else
	{
		// partial volume
		vol = 0xbe;
	}

	song->vol_Curr = vol;
	song->vol_New = vol;

	// Naughty Dog Box
	if (sdata->gGT->levelID == NAUGHTY_DOG_CRATE)
	{
		song->vol_Curr = 0xbe;
		song->vol_New = 0xbe;
	}

	song->vol_StepRate = 1;
	song->numSequences = 0;

	u16 *seqOffsetArr = (u16 *)SONGHEADER_GETSEQOFFARR(csh);

	// first note header comes after end of CseqSongHeader
	// and the full array of seqOffsets within the header
	cnhFirst = (char *)&seqOffsetArr[numSeqs];

	// align up by 4
	if (((uintptr_t)cnhFirst & 1) != 0)
		cnhFirst += 1;
	if (((uintptr_t)cnhFirst & 2) != 0)
		cnhFirst += 2;

	for (i = 0; i < numSeqs; i++)
	{
		cnhCurr = (struct SongNoteHeader *)&cnhFirst[seqOffsetArr[i]];

		seqCurr = SongPool_FindFreeChannel();
		if (seqCurr == NULL)
			continue;

		// now playing
		seqCurr->flags = 1;

		if ((cnhCurr->flags & 1) != 0)
		{
			// playing, and instrument/drums
			seqCurr->flags = 5;
		}

		if (boolLoopAtEnd != 0)
		{
			seqCurr->flags |= 2;
		}

		seqCurr->unk = cnhCurr->unk;

		seqCurr->instrumentID = 0;
		seqCurr->reverb = 0;

		// by default, have volume
		seqCurr->vol_Curr = 0xFF;
		seqCurr->vol_New = 0xFF;

		// songSet (advHub)
		if (songSet != 0)
		{
			// if seq is not in current songSet
			if ((songSet->ptrSongSetBits[i] & song->songSetActiveBits) == 0)
			{
				// disable the volume from the song
				seqCurr->vol_Curr = 0;
				seqCurr->vol_New = 0;
			}
		}

		seqCurr->vol_StepRate = 1;

		seqCurr->distort = 0x80;
		seqCurr->LR = 0x80;

		seqCurr->unk0A = 0;
		seqCurr->songPoolIndex = song->songPoolIndex;

		seqCurr->NoteLength = 0;
		seqCurr->NoteTimeElapsed = 0;

		seqCurr->firstNote = (char *)NOTEHEADER_GETNOTES(cnhCurr);

		seqCurr->currNote = DECOMP_howl_GetNextNote(seqCurr->firstNote, &seqCurr->NoteLength);

		song->CseqSequences[song->numSequences++] = seqCurr;
	}
}
