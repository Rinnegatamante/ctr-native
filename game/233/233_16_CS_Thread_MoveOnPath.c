#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ade8c-0x800ae2b8
void CS_Thread_MoveOnPath(struct Thread *t)
{
	struct CutsceneObj *cs = t->object;
	struct Instance *inst = t->inst;
	struct Level *level;
	struct GameTracker *gGT;
	s16 modelID;
	int switchVal;
	int digit;
	struct SpawnType2 *spawnEntry;
	s16 *coords;
	s16 *curr;
	s16 *next;
	u16 progress;
	int idx;
	u16 frac;
	SVec3 rot;

	if ((cs->flags & CS_FLAG_PATH_MOTION_DISABLED) != 0)
		return;

	if (inst == 0)
		return;

	modelID = inst->model->id;
	switchVal = (s16)(modelID - 0xA1);

	if ((u32)switchVal >= 63)
		return;

	gGT = sdata->gGT;
	level = gGT->level1;

	switch (switchVal)
	{
	case 0x00:
	case 0x3E:

		digit = (u8)inst->name[strlen(inst->name) - 1] - '0';

		if (level->numSpawnType2 <= digit)
			return;

		spawnEntry = (struct SpawnType2 *)((char *)level->ptrSpawnType2 + digit * 8);
		coords = spawnEntry->posCoords;

		if (coords == 0)
			return;

		progress = cs->pathProgress32;
		idx = (s16)progress >> 5;
		cs->pathProgress32 = (u16)(progress + (u16)gGT->elapsedTimeMS);
		frac = progress & 0x1f;

		if (idx >= spawnEntry->numCoords - 1)
		{
			idx = 0;

			if (modelID == 0xDF)
			{
				idx = spawnEntry->numCoords - 2;
				cs->pathProgress32 = idx << 5;
			}
			else
			{
				cs->pathProgress32 = 0;
			}
		}

		curr = &coords[idx * 3];
		next = &curr[3];

		inst->matrix.t[0] = curr[0] + ((frac * (next[0] - curr[0])) >> 5);
		inst->matrix.t[1] = curr[1] + ((frac * (next[1] - curr[1])) >> 5);
		inst->matrix.t[2] = curr[2] + ((frac * (next[2] - curr[2])) >> 5);

		if (idx >= spawnEntry->numCoords - 1)
			return;

		if (modelID == 0xDF)
			return;

		rot.x = cs->rot.x;
		rot.y = cs->rot.y + ratan2(next[0] - curr[0], next[2] - curr[2]);
		rot.z = cs->rot.z;

		ConvertRotToMatrix(&inst->matrix, &rot);
		return;

	case 0x01:
	case 0x02:
	case 0x39:
	case 0x3A:

		digit = (u8)inst->name[strlen(inst->name) - 1] - '0';

		if (level->numSpawnType2_PosRot <= digit)
			return;

		spawnEntry = (struct SpawnType2 *)((char *)level->ptrSpawnType2_PosRot + digit * 8);
		coords = spawnEntry->posCoords;

		if (coords == 0)
			return;

		progress = cs->pathProgress32;
		cs->pathProgress32 = (u16)(progress + (u16)gGT->elapsedTimeMS);
		idx = (s16)progress >> 5;

		if (idx >= spawnEntry->numCoords - 1)
		{
			idx = 0;
			cs->pathProgress32 = 0;
		}

		{
			s16 *point = &coords[idx * 6];

			inst->matrix.t[0] = point[0];
			inst->matrix.t[1] = point[1];
			inst->matrix.t[2] = point[2];

			rot.x = point[3];
			rot.y = point[4];
			rot.z = point[5];
		}

		break;

	case 0x30:

		if (level->numSpawnType2 <= 0)
			return;

		spawnEntry = level->ptrSpawnType2;
		coords = spawnEntry->posCoords;

		if (coords == 0)
			return;

		{
			int prog = 0;

			if (cs->animIndex == 3)
				prog = cs->unk18;

			frac = prog & 0x1f;
			int numCoords = spawnEntry->numCoords;
			idx = prog >> 5;

			if (idx < numCoords - 1)
			{
				if (idx >= 0)
				{
					curr = &coords[idx * 3];
					next = &curr[3];
				}
				else
				{
					curr = &coords[0];
					next = curr;
				}
			}
			else
			{
				curr = &coords[(numCoords - 1) * 3];
				next = curr;
			}

			inst->matrix.t[0] = curr[0] + ((frac * (next[0] - curr[0])) >> 5);
			inst->matrix.t[1] = curr[1] + ((frac * (next[1] - curr[1])) >> 5);
			inst->matrix.t[2] = curr[2] + ((frac * (next[2] - curr[2])) >> 5);
		}

		return;

	default:
		return;
	}

	ConvertRotToMatrix(&inst->matrix, &rot);
}
