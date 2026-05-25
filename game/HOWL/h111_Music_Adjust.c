#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e350-0x8002e418
// param_1 - SongID (playing)
// param_2 - deltaBPM
// param_3 - 8008d068 for AdvHub
// param_4 - songSetActiveBits
void Music_Adjust(u32 songID, int newTempo, struct SongSet *set, u32 songSetActiveBits)
{
	songID &= 0xffff;

	// if cseq music can play
	if (sdata->cseqBoolPlay != 0)
	{
		if (sdata->cseqHighestIndex == songID)
		{
			// if tempo has changed
			if (sdata->cseqTempo != newTempo)
			{
				CseqMusic_ChangeTempo(sdata->cseqHighestIndex, newTempo);
				sdata->cseqTempo = newTempo;
			}
		}
		else
		{
			CseqMusic_Stop(sdata->cseqHighestIndex);
		}
	}

	// if new SongID
	if (sdata->cseqHighestIndex != songID)
	{
		//  (loopAtEnd)
		CseqMusic_Start(songID, newTempo, set, songSetActiveBits, 1);

		sdata->cseqBoolPlay = true;

		// set active SongID and tempo
		sdata->cseqHighestIndex = songID;
		sdata->cseqTempo = newTempo;
	}
}
