#include <common.h>

static const s16 sparkGround_inX[4] = {0x1800, 0, 0, 0};
static const s16 sparkGround_inZ[4] = {0, 0, -0x1800, 0};
static const s16 sparkGround_inZ2[4] = {0, 0, -0x200, 0};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80059344-0x80059558
void VehEmitter_Sparks_Ground(struct Driver *d, struct ParticleEmitter *emSet)
{
	struct GameTracker *gGT = sdata->gGT;

	int outX[3];
	int outZ[3];
	int outZ2[3];

	gte_ldv0(sparkGround_inX);
	gte_rtv0();
	gte_stlvnl(outX);

	gte_ldv0(sparkGround_inZ);
	gte_rtv0();
	gte_stlvnl(outZ);

	gte_ldv0(sparkGround_inZ2);
	gte_rtv0();
	gte_stlvnl(outZ2);

	for (int i = 0; i < 10; i++)
	{
		// Create instance in particle pool
		struct Particle *p = Particle_Init(0, gGT->iconGroup[0], emSet);

		if (p == NULL)
			continue;

		u32 rng = (u32)(RngDeadCoed(&gGT->deadcoed_struct.unk1) & 0x7ff);

		if ((rng & 1) != 0)
		{
			rng = -rng;
		}

		for (int j = 0; j < 3; j++)
		{
			p->axis[j].velocity += (s16)outZ2[j] + (s16)((rng * outX[j]) >> 12);
			p->axis[j].startVal += (int)outZ[j] + p->axis[j].velocity;
		}

		p->driverInst = d->instSelf;
		p->unk18 = d->instSelf->unk50;
	}
}
