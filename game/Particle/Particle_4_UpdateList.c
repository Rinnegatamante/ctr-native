#include <common.h>

static u32 Particle_GetAxisFlags(const struct Particle *p)
{
	return *(const u32 *)&p->flagsAxis;
}

static void Particle_SetAxisFlags(struct Particle *p, u32 flags)
{
	*(u32 *)&p->flagsAxis = flags;
}

static int Particle_OscillatorValue(struct ParticleOscillator *osc)
{
	int value;
	int timer = sdata->gGT->frameTimer_Confetti;
	int phase = timer + osc->phase;
	int product = (int)osc->period * phase;

	switch (osc->flags & 7)
	{
	case 0:
		value = MATH_Sin(product >> 5);
		break;

	case 1:
		value = MATH_Sin(product >> 6);
		if (value < 0)
			value = -value;
		value = (value << 1) - 0x1000;
		break;

	case 2:
		value = ((product >> 4) & 0x1fff) - 0x1000;
		break;

	case 3:
		value = (product >> 3) & 0x3fff;
		if (value > 0x2000)
			value = 0x4000 - value;
		value -= 0x1000;
		break;

	case 4:
		value = -0x1000;
		if (((product >> 6) & 0x400) != 0)
			value = 0x1000;
		break;

	case 5:
		value = (MixRNG_Scramble() >> 3) - 0x1000;
		break;

	case 6:
		value = ((int)MixRNG_GetValue((s16)osc->previousValue) >> 3) - 0x1000;
		break;

	default:
		value = timer;
		break;
	}

	value = ((value + osc->offset) * (int)osc->scale) >> 12;

	if (value > osc->max)
		value = osc->max;
	if (value < osc->min)
		value = osc->min;

	return value;
}

static void Particle_ApplyOscillator(struct ParticleAxis *axis, struct ParticleOscillator *osc)
{
	int value;

	if ((osc->flags & 8) == 0)
	{
		if ((osc->flags & 0x10) == 0)
			axis->startVal -= (s16)osc->previousValue;
		else
			axis->velocity = (s16)(axis->velocity - osc->previousValue);
	}

	value = Particle_OscillatorValue(osc);

	if ((osc->flags & 0x10) == 0)
		axis->startVal += value;
	else
		axis->velocity = (s16)(axis->velocity + value);

	osc->previousValue = (s16)value;
}

static int Particle_ColorExpired(struct Particle *p, u16 activeFlags)
{
	int value = 0;

	if ((activeFlags & 0x80) != 0 && p->axis[7].startVal > 0)
		value = p->axis[7].startVal;

	if ((activeFlags & 0x100) != 0 && p->axis[8].startVal > 0)
		value |= p->axis[8].startVal;

	if ((activeFlags & 0x200) != 0 && p->axis[9].startVal > 0)
		value |= p->axis[9].startVal;

	return value < 0x800;
}

static void Particle_UpdateIconFrame(struct Particle *p, u16 flagsSetColor)
{
	int frame = p->axis[10].startVal;
	int frameLimit = p->ptrIconGroup->numIcons << 8;

	if (frame < 0)
	{
		if ((flagsSetColor & 0x100) != 0)
		{
			frame += frameLimit;
		}
		else if ((flagsSetColor & 0x200) != 0)
		{
			frame -= p->axis[10].velocity * 2;
			p->axis[10].accel = -p->axis[10].accel;
			p->axis[10].velocity = -p->axis[10].velocity;
		}
		else
		{
			frame = 0;
		}
	}
	else
	{
		if (frame < frameLimit)
			return;

		if ((flagsSetColor & 0x100) != 0)
		{
			frame -= frameLimit;
		}
		else if ((flagsSetColor & 0x200) != 0)
		{
			frame -= p->axis[10].velocity * 2;
			p->axis[10].accel = -p->axis[10].accel;
			p->axis[10].velocity = -p->axis[10].velocity;
		}
		else
		{
			frame = frameLimit - 1;
		}
	}

	p->axis[10].startVal = frame;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003eefc-0x8003f434
void Particle_UpdateList(struct Particle **listHead, struct Particle *p)
{
	struct Particle **link = listHead;

	while (p != NULL)
	{
		struct Particle *next = p->next;
		u16 flagsSetColor;
		u32 axisFlags;
		u16 activeFlags;
		struct ParticleOscillator *osc;

		p->framesLeftInLife = (s16)(p->framesLeftInLife - 1);
		if (p->framesLeftInLife == -1)
			goto destroyParticle;

		flagsSetColor = p->flagsSetColor;
		if ((flagsSetColor & 8) != 0)
			goto destroyParticle;

		if ((flagsSetColor & 0x1000) != 0)
		{
			p->axis[3].startVal = p->axis[0].startVal;
			p->axis[6].startVal = p->axis[1].startVal;
			p->axis[4].startVal = p->axis[2].startVal;

			if ((flagsSetColor & 0x4000) == 0)
				*(u32 *)&p->axis[10].startVal = *(u32 *)&p->axis[10].velocity;
		}

		axisFlags = Particle_GetAxisFlags(p);
		osc = p->oscillator;

		for (int axisIndex = 0; axisFlags != 0; axisIndex++)
		{
			if ((axisFlags & 1) != 0)
			{
				struct ParticleAxis *axis = &p->axis[axisIndex];

				axis->startVal += axis->velocity;
				axis->velocity = (s16)(axis->velocity + axis->accel);

				if (((axisFlags >> 16) & 1) != 0 && osc != NULL)
				{
					Particle_ApplyOscillator(axis, osc);
					osc = osc->next;
				}
			}

			axisFlags = (axisFlags & 0xfffeffffu) >> 1;
		}

		if (p->funcPtr != NULL)
		{
			void (*funcPtr)(struct Particle *) = (void (*)(struct Particle *))p->funcPtr;
			funcPtr(p);
		}

		activeFlags = p->flagsAxis;

		if ((flagsSetColor & 1) != 0)
		{
			if (((activeFlags & 0x20) != 0 && p->axis[5].startVal < 1) || ((activeFlags & 0x40) != 0 && p->axis[6].startVal < 1))
				goto destroyParticle;
		}

		if ((flagsSetColor & 2) != 0 && Particle_ColorExpired(p, activeFlags))
			goto destroyParticle;

		link = &p->next;

		if ((activeFlags & 0x400) != 0 && p->ptrIconGroup != NULL)
			Particle_UpdateIconFrame(p, flagsSetColor);

		p = next;
		continue;

	destroyParticle:
		Particle_OnDestroy(p);
		LIST_AddFront(&sdata->gGT->JitPools.particle.free, (struct Item *)p);
		sdata->gGT->numParticles--;
		*link = next;
		p = next;
	}
}
