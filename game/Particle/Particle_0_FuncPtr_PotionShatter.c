#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003eae0-0x8003ec18
void Particle_FuncPtr_PotionShatter(struct Particle *p)
{
	s16 sVar2;
	int rng;

	if (p->axis[1].velocity < 0x578)
	{
		if (p->axis[0].velocity != 0)
			goto LAB_8003ebc8;

		// random X
		rng = MixRNG_Scramble();
		p->axis[0].velocity = rng + (rng / 800) * -800 - 400;

		// random Z
		rng = MixRNG_Scramble();
		p->axis[2].velocity = rng + (rng / 800) * -800 - 400;

		// random scale
		rng = MixRNG_Scramble();
		sVar2 = (rng >> 8);
		if (rng < 0)
		{
			sVar2 = ((rng + 0xff) >> 8);
		}
		p->axis[5].velocity = rng + sVar2 * -0x100 + 0x100;
	}
	if (p->axis[0].velocity == 0)
		return;

LAB_8003ebc8:

	// green shatter or red shatter
	if (p->modelID == STATIC_SHOCKWAVE_GREEN)
	{
		if (0 < p->axis[8].startVal)
		{
			p->axis[8].startVal -= 0x1200;
		}
	}
	else
	{
		if (0 < p->axis[7].startVal)
		{
			p->axis[7].startVal -= 0x1200;
		}
	}
}
