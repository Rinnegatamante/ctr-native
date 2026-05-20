#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003b008-0x8003b0f0
void MainInit_RainBuffer(struct GameTracker *gGT)
{
	u8 numPlyr = gGT->numPlyrCurrGame;

	if (numPlyr == 0)
		return;

	for (int i = 0; i < numPlyr; i++)
	{
		struct RainBuffer *dst = &gGT->rainBuffer[i];
		const u32 *srcWords = (const u32 *)(const void *)&gGT->level1->rainBuffer;
		u32 *dstWords = (u32 *)(void *)dst;

		for (int word = 0; word < (int)(sizeof(struct RainBuffer) / sizeof(u32)); word += 4)
		{
			dstWords[word + 0] = srcWords[word + 0];
			dstWords[word + 1] = srcWords[word + 1];
			dstWords[word + 2] = srcWords[word + 2];
			dstWords[word + 3] = srcWords[word + 3];
		}

		dst->numParticles_curr /= numPlyr;
		dst->numParticles_max = (s16)((u16)dst->numParticles_max / numPlyr);
	}
}
