#include <common.h>

void RB_Explosion_ThTick();

static const u32 s_potionShatterEmitter[] = {
    0x000c0001, 0x00000000, 0x001400a1, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000001, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00020001, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00010017, 0x00000001, 0xfee80ed8, 0x00000000, 0x00000190, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00050001, 0x00001000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00070001, 0x00000001, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00080001, 0x0000c800, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00090001, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b1458-0x800b1630.
// NOTE(aalhendi): Native uses retail emitter bytes from 0x800b2d58.
void RB_Explosion_InitPotion(struct Instance *inst)
{
	struct Instance *shatterInst;
	struct Particle *p;
	int shatterColor;

	// green explosion
	shatterColor = STATIC_SHOCKWAVE_GREEN;

	// if red beaker, red explosion
	if (inst->model->id == STATIC_BEAKER_RED)
		shatterColor = STATIC_SHOCKWAVE_RED;

	// create thread for shatter
	shatterInst = INSTANCE_BirthWithThread(shatterColor, 0, SMALL, OTHER, RB_Explosion_ThTick, 0, 0);

	shatterInst->flags |= 0xa00;

	// set funcThDestroy to remove instance from instance pool
	shatterInst->thread->funcThDestroy = PROC_DestroyInstance;

	// copy position and rotation from one instance to the other
	*(int *)&shatterInst->matrix.m[0][0] = *(int *)&inst->matrix.m[0][0];
	*(int *)&shatterInst->matrix.m[0][2] = *(int *)&inst->matrix.m[0][2];
	*(int *)&shatterInst->matrix.m[1][1] = *(int *)&inst->matrix.m[1][1];
	*(int *)&shatterInst->matrix.m[2][0] = *(int *)&inst->matrix.m[2][0];
	shatterInst->matrix.m[2][2] = inst->matrix.m[2][2];

	for (int i = 0; i < 3; i++)
	{
		shatterInst->scale[i] = 0x800;
		shatterInst->matrix.t[i] = inst->matrix.t[i];
	}

	// particles for potion shatter
	for (int i = 0; i < 5; i++)
	{
		// Create instance in particle pool
		p = Particle_Init(0, sdata->gGT->iconGroup[1], (struct ParticleEmitter *)s_potionShatterEmitter);

		if (p == NULL)
			continue;

		p->axis[0].startVal += shatterInst->matrix.t[0] * 0x100;
		p->axis[1].startVal += shatterInst->matrix.t[1] * 0x100;
		p->axis[2].startVal += shatterInst->matrix.t[2] * 0x100;

		p->modelID = shatterColor;

		if (shatterColor == STATIC_SHOCKWAVE_GREEN)
		{
			p->axis[7].startVal = 1;
			p->axis[8].startVal = 0xc800;
		}

		else
		{
			p->axis[7].startVal = 0xc800;
			p->axis[8].startVal = 1;
		}

		p->axis[9].startVal = 1;

		void Particle_FuncPtr_PotionShatter();
		p->funcPtr = Particle_FuncPtr_PotionShatter;
	}

	RB_Potion_OnShatter_TeethSearch(inst);
	return;
}
