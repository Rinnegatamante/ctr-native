#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b06ac-0x800b087c
void CS_Thread_LInB(struct Instance *inst)
{
	struct Thread *t;
	struct CutsceneObj *cs;
	s16 modelID;
	char *scriptPtr;

	D233.isCutsceneOver = 0;

	if (inst->thread != 0)
		goto check_polar;

	t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(0x60, NONE, MEDIUM, STATIC), CS_Thread_ThTick, R233.s_introguy, 0);

	inst->thread = t;

	if (t == 0)
		return;

	cs = t->object;

	t->inst = inst;

	cs->metadata = (int *)&cs->decodedOpcode;
	cs->prevOpcode = (char *)-1;
	cs->Subtitles.lngIndex = -1;

	modelID = inst->model->id;

	if (modelID < NDI_BOX_BOX_01)
	{
		if ((u16)(modelID - STATIC_CRASHINTRO) < 0x10)
		{
			scriptPtr = R233.introModelScripts[modelID - STATIC_CRASHINTRO];
		}
		else
		{
			scriptPtr = (char *)R233.script_default;
		}
	}
	else
	{
		scriptPtr = R233.boxModelScripts[modelID - NDI_BOX_BOX_01];
	}

	CS_ScriptCmd_OpcodeAt(cs, scriptPtr);

	cs->unk18 = *(int *)(cs->metadata + 2);

	{
		int rng = MixRNG_Scramble();
		s16 *meta = (s16 *)cs->metadata;
		s16 frameStart = meta[2];
		s16 frameEnd = meta[3];

		cs->unk1c = 0;
		cs->rot.x = 0;
		cs->rot.y = 0;
		cs->rot.z = 0;
		cs->pathProgress32 = 0;
		cs->unk1e = 0;
		cs->flags = 0;
		cs->scaleSpeed = 0;
		cs->frameOverrideRoot = 0;
		cs->desiredScale = 0x1000;
		cs->particleID = 0xff;

		cs->opcodeDuration = frameStart + (s16)(((rng >> 2 & 0xfff) * ((frameEnd - frameStart) + 1)) >> 0xc);

		struct GameTracker *gGT = sdata->gGT;

		cs->unk4 = 0;
		cs->unk6 = 0;
		cs->unk8 = 0x2e808080;
		cs->unk_C = 0;
		cs->unk_E = 0;

		cs->ptrIcons = (struct IconGroup *)((char *)gGT->iconGroup[0] + sizeof(struct IconGroup));
	}

check_polar:
	if (sdata->gGT->levelID == 0x21)
	{
		inst->vertSplit = 0;
		inst->flags |= REFLECTIVE;
	}
}
