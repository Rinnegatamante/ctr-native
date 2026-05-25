#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002ad70-0x8002ae64
void UpdateChannelVol_Music(struct SongSeq *songSeq, struct ChannelAttr *attr, int index, int vol)
{
	int sampleVol;

	int newVol = (sdata->vol_Music * sdata->songPool[songSeq->songPoolIndex].vol_Curr * songSeq->vol_Curr) >> 10;

	if ((songSeq->flags & 4) == 0)
	{
		sampleVol = sdata->ptrCseqLongSamples[songSeq->instrumentID].volume;
	}

	else
	{
		sampleVol = sdata->ptrCseqShortSamples[index].volume;
	}

	Channel_SetVolume(attr, (newVol * sampleVol * vol) >> 0xf, songSeq->LR);
}
