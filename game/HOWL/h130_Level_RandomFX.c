#include <common.h>

static u32 Level_RandomFX_NextAudioRNG(void)
{
	sdata->audioRNG = ((sdata->audioRNG >> 3) + sdata->audioRNG * 0x20000000) * 5 + 1;
	return sdata->audioRNG;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002eab8-0x8002ebe4
void Level_RandomFX(int *cooldown, u32 soundID, int baseCooldown, u32 randomRange, int volumeScale)
{
	int cooldownValue = *cooldown;

	if (cooldownValue > 0)
	{
		*cooldown = cooldownValue - 1;
		cooldownValue = *cooldown;
	}

	if (cooldownValue == 0)
	{
		u32 rng = Level_RandomFX_NextAudioRNG();

		DECOMP_OtherFX_Play_LowLevel(soundID & 0xffff, 0, ((((rng % 100 + 100) * volumeScale >> 8) & 0xff) << 0x10) | 0x1008080);

		rng = Level_RandomFX_NextAudioRNG();
		*cooldown = rng % randomRange + baseCooldown;
	}
}
