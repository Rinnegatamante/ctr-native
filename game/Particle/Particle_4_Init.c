#include <common.h>

static u32 Particle_Init_GetAxisFlags(const struct Particle *p)
{
	return *(const u32 *)&p->flagsAxis;
}

static void Particle_Init_SetAxisFlags(struct Particle *p, u32 flags)
{
	*(u32 *)&p->flagsAxis = flags;
}

static u8 ParticleEmitter_GetInitOffset(const struct ParticleEmitter *emSet)
{
	return ((const u8 *)emSet)[2];
}

static void ParticleEmitter_CopyOscillator(struct ParticleOscillator *osc, const struct ParticleEmitter *emSet)
{
	const u32 *src = (const u32 *)emSet->data;
	u32 *dst = (u32 *)&osc->flags;

	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

static void Particle_InitAxis(struct Particle *p, const struct ParticleEmitter *emSet, u8 axisIndex, u32 *flagsAxis)
{
	struct ParticleAxis *axis = &p->axis[axisIndex];
	u16 flags = emSet->flags;
	int value = 0;
	s16 velocity = 0;
	s16 accel = 0;

	if ((flags & 1) != 0)
		value = emSet->InitTypes.AxisInit.baseValue.startVal;
	if ((flags & 8) != 0)
		value += MixRNG_Particles(emSet->InitTypes.AxisInit.rngSeed.startVal);

	axis->startVal = value;

	if ((flags & 2) != 0)
		velocity = emSet->InitTypes.AxisInit.baseValue.velocity;
	if ((flags & 0x10) != 0)
		velocity = (s16)(velocity + MixRNG_Particles(emSet->InitTypes.AxisInit.rngSeed.velocity));

	axis->velocity = velocity;

	if ((flags & 4) != 0)
		accel = emSet->InitTypes.AxisInit.baseValue.accel;
	if ((flags & 0x20) != 0)
		accel = (s16)(accel + MixRNG_Particles(emSet->InitTypes.AxisInit.rngSeed.accel));

	axis->accel = accel;

	*flagsAxis |= 1u << (axisIndex & 0x1f);
}

static void Particle_InitOscillator(struct Particle *p, struct ParticleOscillator *localOsc[12], const struct ParticleEmitter *emSet, u8 axisIndex,
                                    u32 *flagsAxis)
{
	struct ParticleOscillator *osc;
	u32 oscBit = 1u << ((axisIndex + 0x10) & 0x1f);
	u32 axisBit = 1u << (axisIndex & 0x1f);

	if ((*flagsAxis & oscBit) == 0)
	{
		osc = (struct ParticleOscillator *)LIST_RemoveFront(&sdata->gGT->JitPools.oscillator.free);
		if (osc == NULL)
			return;

		localOsc[axisIndex] = osc;
	}
	else
	{
		osc = localOsc[axisIndex];
	}

	ParticleEmitter_CopyOscillator(osc, emSet);

	if ((osc->flags & 0x20) != 0)
		osc->phase = (s16)(osc->phase - (u16)sdata->gGT->frameTimer_Confetti);

	if ((osc->flags & 7) == 6)
		osc->previousValue = osc->phase;

	*flagsAxis |= oscBit;

	if ((*flagsAxis & axisBit) == 0)
	{
		struct ParticleAxis *axis = &p->axis[axisIndex];

		*flagsAxis |= axisBit;
		axis->startVal = 0;
		axis->velocity = 0;
		axis->accel = 0;
	}
}

static void Particle_RandomizeOscillator(struct ParticleOscillator *localOsc[12], const struct ParticleEmitter *emSet, u8 axisIndex, u32 flagsAxis)
{
	struct ParticleOscillator *osc;
	const s16 *rng;

	if ((flagsAxis & (1u << ((axisIndex + 0x10) & 0x1f))) == 0)
		return;

	osc = localOsc[axisIndex];
	rng = (const s16 *)&((const u8 *)emSet)[0x18];

	if ((u16)rng[0] != 0)
		osc->period = (u16)(osc->period + MixRNG_Particles((u16)rng[0]));
	if (rng[1] != 0)
		osc->phase = (s16)(osc->phase + MixRNG_Particles(rng[1]));
	if ((u16)rng[2] != 0)
		osc->scale = (u16)(osc->scale + MixRNG_Particles((u16)rng[2]));
	if (rng[3] != 0)
		osc->offset = (s16)(osc->offset + MixRNG_Particles(rng[3]));
	if (rng[4] != 0)
		osc->min = (s16)(osc->min + MixRNG_Particles(rng[4]));
	if (rng[5] != 0)
		osc->max = (s16)(osc->max + MixRNG_Particles(rng[5]));
}

static void Particle_LinkOscillators(struct Particle *p, struct ParticleOscillator *localOsc[12], u32 flagsAxis)
{
	struct ParticleOscillator **link = &p->oscillator;
	u32 oscFlags = (s32)flagsAxis >> 16;
	int axisIndex = 0;

	while (oscFlags != 0)
	{
		if ((oscFlags & 1) != 0)
		{
			*link = localOsc[axisIndex];
			link = &localOsc[axisIndex]->next;
		}

		oscFlags = (s32)oscFlags >> 1;
		axisIndex++;
	}

	*link = NULL;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80040308-0x80040850
struct Particle *Particle_Init(u32 param_1, struct IconGroup *ig, struct ParticleEmitter *emSet)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Particle *p;
	struct ParticleOscillator *localOsc[12];
	u32 flagsAxis = 0;
	int particleType = 0;

	(void)param_1;

	p = (struct Particle *)LIST_RemoveFront(&gGT->JitPools.particle.free);
	if (p == NULL)
		return NULL;

	gGT->numParticles++;

	p->ptrIconGroup = ig;
	if (ig != NULL && ig->numIcons != 0 && ig->numIcons > 0)
		p->ptrIconArray = ((struct Icon **)ICONGROUP_GETICONS(ig))[0];
	else
	{
		p->ptrIconGroup = NULL;
		p->ptrIconArray = NULL;
	}

	if (emSet != NULL)
	{
		while (emSet->flags != 0)
		{
			u16 flags = emSet->flags;
			u8 axisIndex = ParticleEmitter_GetInitOffset(emSet);

			if (axisIndex == 0xc)
			{
				if ((flags & 0xc0) == 0)
				{
					p->funcPtr = emSet->InitTypes.FuncInit.particle_funcPtr;
					p->flagsSetColor = emSet->InitTypes.FuncInit.particle_colorFlags;
					p->framesLeftInLife = emSet->InitTypes.FuncInit.particle_lifespan;
					flagsAxis |= 0x1000;
					particleType = emSet->InitTypes.FuncInit.particle_Type;
				}
			}
			else if ((flags & 0x80) != 0)
			{
				Particle_RandomizeOscillator(localOsc, emSet, axisIndex, flagsAxis);
			}
			else if ((flags & 0x40) != 0)
			{
				Particle_InitOscillator(p, localOsc, emSet, axisIndex, &flagsAxis);
			}
			else
			{
				Particle_InitAxis(p, emSet, axisIndex, &flagsAxis);
			}

			emSet++;
		}
	}

	Particle_LinkOscillators(p, localOsc, flagsAxis);

	if ((flagsAxis & 0x1000) == 0)
	{
		p->funcPtr = NULL;
		p->flagsSetColor = 0;
		p->framesLeftInLife = 0;
		p->ptrIconArray = NULL;
		p->ptrIconGroup = NULL;
	}

	Particle_Init_SetAxisFlags(p, flagsAxis & 0xffffefff);

	if (particleType == 0)
	{
		p->next = gGT->particleList_ordinary;
		gGT->particleList_ordinary = p;
	}
	else
	{
		p->next = gGT->particleList_heatWarp;
		gGT->particleList_heatWarp = p;
	}

	p->unk1A = 0x400;
	p->unk19 = -1;
	p->unk18 = 0;

	if ((p->flagsSetColor & 0x1000) != 0)
	{
		if ((p->flagsSetColor & 0x4000) == 0)
		{
			u32 color = Particle_SetColors(Particle_Init_GetAxisFlags(p), p->flagsSetColor, p) | 0x50000000;

			p->axis[10].startVal = color;
			*(u32 *)&p->axis[10].velocity = color;
		}
		else
		{
			u32 color = 0x50000000;

			if ((p->flagsSetColor & 0x80) != 0)
				color = 0x52000000;

			p->axis[10].startVal = color;
		}
	}

	return p;
}
