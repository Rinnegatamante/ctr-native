#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002a4a8-0x8002a63c
void cseq_opcode0a(struct SongSeq *seq)
{
	int pitch;
	struct ChannelStats *curr, *backupNext;
	int soundID = seq->soundID;

	u8 *currNote = seq->currNote;
	seq->distort = currNote[1];

	for (curr = (struct ChannelStats *)sdata->channelTaken.first; curr != NULL; curr = backupNext)
	{
		backupNext = curr->next;

		// type != MUSIC
		if (curr->type != 2)
			continue;

		// not the sound needed to turn off
		if (curr->soundID != soundID)
			continue;

		int index = curr->drumIndex_pitchIndex;

		// instrument
		if ((seq->flags & 4) == 0)
		{
			struct SampleInstrument *longSample = &sdata->ptrCseqLongSamples[seq->instrumentID];

			pitch = DECOMP_howl_InstrumentPitch(longSample->basePitch, index, seq->distort);
		}

		// drums
		else
		{
			struct SampleDrums *shortSample = &sdata->ptrCseqShortSamples[index];

			if (seq->distort == 0x80)
			{
				pitch = shortSample->pitch;
			}

			else
			{
				pitch = shortSample->pitch * data.distortConst_OtherFX[seq->distort] >> 0x10;
			}
		}

		// save pitch
		sdata->channelAttrNew[curr->channelID].pitch = pitch;

		// update pitch
		sdata->ChannelUpdateFlags[curr->channelID] |= 0x10;
	}
}
