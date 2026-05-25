#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003eeb0-0x8003eefc
void Particle_OnDestroy(struct Particle *p)
{
	struct ParticleOscillator *osc;

	osc = p->oscillator;

	while (osc != NULL)
	{
		struct ParticleOscillator *next = osc->next;

		LIST_AddFront(&sdata->gGT->JitPools.oscillator.free, (struct Item *)osc);
		osc = next;
	}
}
