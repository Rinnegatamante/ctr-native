#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003ec18-0x8003ee20
void Particle_FuncPtr_SpitTire(struct Particle *p)
{
	int rng;
	int iVar2;
	int targetY;

	// Wait until tires are 0x10 units above
	// the ground, which is where the plant
	// actually "spits" tires from the mouth
	targetY = p->plantInst->matrix.t[1] + 0x10;

	if ((p->axis[1].startVal >> 8) >= targetY)
	{
		return;
	}

	// random X
	rng = MixRNG_Scramble();
	p->axis[0].velocity = rng + (rng / 0x1640) * -0x1640 - 0xb20;

	// random Z
	rng = MixRNG_Scramble();
	p->axis[2].velocity = rng + (rng / 0x1640) * -0x1640 - 0xb20;

	// scale value
	iVar2 = p->axis[5].startVal;

	switch (iVar2)
	{
	// frame #1
	case 0x1000:
	{
		// random Y
		rng = MixRNG_Scramble();
		p->axis[1].velocity = rng + (rng / 0x12c0) * -0x12c0 + 0x1900;

		// frame #2
		p->axis[5].startVal = 0xfff;
		break;
	}

	// frame #2
	case 0xfff:
	{
		// random Y
		rng = MixRNG_Scramble();
		p->axis[1].velocity = rng + (rng / 800) * -800 + 8000;

		// frame #3
		p->axis[5].startVal = 0xffe;
		break;
	}

	// frame #3
	case 0xffe:
	{
		// random Y
		rng = MixRNG_Scramble();
		p->axis[1].velocity = rng + (rng / 800) * -800 + 6000;

		p->axis[5].velocity = 0xf801;
		break;
	}

	default:
		return;
	}

	p->axis[1].startVal = targetY * 0x100;
}
