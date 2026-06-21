#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abdd4-0x800abf70
void CS_Thread_Particles(struct Thread *t)
{
	struct CutsceneObj *cs = t->object;
	struct Instance *inst = t->inst;
	const struct CsParticleConfig *entry;
	s8 particleID;

	if (inst == NULL)
		return;

	if ((inst->flags & HIDE_MODEL) != 0)
		return;

	particleID = cs->particleID;
	if ((u8)particleID >= 9)
		return;

	entry = &R233.particleConfigs[(int)particleID];

	while (1)
	{
		int iconGroupIndex = entry->meta.iconGroupIndex;
		int frameOffset = entry->meta.frameOffset;
		int count = entry->meta.count;
		int flags = entry->meta.flags;
		s8 modelDelta = entry->spawn.modelDelta;

		for (int i = 0; i < count; i++)
		{
			struct Particle *p = Particle_Init(0, sdata->gGT->iconGroup[iconGroupIndex], entry->emitter);

			if (p != NULL)
			{
				SVec3 pos;

				CS_Instance_GetFrameData(inst, inst->animIndex, inst->animFrame, (u16 *)pos.v, NULL, frameOffset);

				p->axis[0].startVal += (pos.x + inst->matrix.t[0]) << 8;
				p->axis[1].startVal += (pos.y + inst->matrix.t[1]) << 8;
				p->axis[2].startVal += (pos.z + inst->matrix.t[2]) << 8;
				p->otIndexOffset = inst->depthBiasNormal + modelDelta;
			}
		}

		if ((flags & 1) == 0)
			break;

		entry++;
	}
}
