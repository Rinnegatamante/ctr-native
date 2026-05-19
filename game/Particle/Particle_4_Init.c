#include <common.h>

static int Particle_Init_Rand(short range)
{
	if (range == 0)
		return 0;

	return DECOMP_MixRNG_Scramble() % range;
}

struct Particle *Particle_Init(u_int param_1, struct IconGroup *ig, struct ParticleEmitter *emSet)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Particle *p;
	int particleType = 0;

	(void)param_1;

	p = (struct Particle *)DECOMP_LIST_RemoveFront(&gGT->JitPools.particle.free);
	if (p == NULL)
		return NULL;

	for (int i = 0; i < (int)(sizeof(*p) / sizeof(int)); i++)
		((int *)p)[i] = 0;

	gGT->numParticles++;

	p->ptrIconGroup = ig;
	if (ig != NULL && ig->numIcons > 0)
		p->ptrIconArray = (struct Icon *)ICONGROUP_GETICONS(ig);

	p->framesLeftInLife = 0x20;

	while (emSet != NULL && emSet->flags != 0)
	{
		u_short flags = emSet->flags;
		int axisIndex = emSet->initOffset;

		if (axisIndex == 12)
		{
			if ((flags & 0xc0) == 0)
			{
				p->funcPtr = emSet->InitTypes.FuncInit.particle_funcPtr;
				p->flagsSetColor = emSet->InitTypes.FuncInit.particle_colorFlags;
				p->framesLeftInLife = emSet->InitTypes.FuncInit.particle_lifespan;
				particleType = emSet->InitTypes.FuncInit.particle_Type;
			}
		}
		else if (axisIndex >= 0 && axisIndex < 0xb && (flags & 0xc0) == 0)
		{
			struct ParticleAxis *axis = &p->axis[axisIndex];

			if ((flags & 1) != 0)
				axis->startVal = emSet->InitTypes.AxisInit.baseValue.startVal;
			if ((flags & 8) != 0)
				axis->startVal += Particle_Init_Rand(emSet->InitTypes.AxisInit.rngSeed.startVal);

			if ((flags & 2) != 0)
				axis->velocity = emSet->InitTypes.AxisInit.baseValue.velocity;
			if ((flags & 0x10) != 0)
				axis->velocity += Particle_Init_Rand(emSet->InitTypes.AxisInit.rngSeed.velocity);

			if ((flags & 4) != 0)
				axis->accel = emSet->InitTypes.AxisInit.baseValue.accel;
			if ((flags & 0x20) != 0)
				axis->accel += Particle_Init_Rand(emSet->InitTypes.AxisInit.rngSeed.accel);
		}

		emSet++;
	}

	{
		struct Particle **head = (particleType == 1) ? (struct Particle **)&gGT->particleList_heatWarp : (struct Particle **)&gGT->particleList_ordinary;

		p->prev = NULL;
		p->next = *head;
		if (p->next != NULL)
			p->next->prev = p;
		*head = p;
	}

	return p;
}
