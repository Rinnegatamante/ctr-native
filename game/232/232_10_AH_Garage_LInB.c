#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af070-0x800af3a4.
void AH_Garage_LInB(struct Instance *inst)
{
	char bossIsOpen, i;
	s16 *check;
	u32 bitIndex;
	int levelID;
	int ratio;
	struct AdvProgress *adv;
	struct Thread *t;
	struct Instance *garageTop;
	struct BossGarageDoor *garage;
	struct GameTracker *gGT;

	gGT = sdata->gGT;
	adv = &sdata->advProgress;
	bossIsOpen = true;
	levelID = gGT->levelID;

	if (inst->thread != NULL)
		return;

	t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct BossGarageDoor), NONE, SMALL, STATIC),

	                         AH_Garage_ThTick, // behavior
	                         R232.s_garage,    // debug name
	                         0                 // thread relative
	);

	if (t == NULL)
		return;

	inst->thread = t;
	t->inst = inst;
	t->funcThDestroy = AH_Garage_ThDestroy;

	garage = t->object;
	garage->direction = 0;
	garage->cooldown = 0;

	// if it is Oxide's Door
	if (inst->model->id == STATIC_OXIDEGARAGE)
	{
		garage->garageTopInst = NULL;
	}

	// if this is not oxide's door
	else
	{
		// make a "garagetop" to make door appear to roll up

		garageTop = INSTANCE_Birth3D(gGT->modelPtr[STATIC_GARAGETOP], R232.s_garagetop, t);

		// copy matrix from one instance to the other
		*(int *)&garageTop->matrix.m[0][0] = *(int *)&inst->matrix.m[0][0];
		*(int *)&garageTop->matrix.m[0][2] = *(int *)&inst->matrix.m[0][2];
		*(int *)&garageTop->matrix.m[1][1] = *(int *)&inst->matrix.m[1][1];
		*(int *)&garageTop->matrix.m[2][0] = *(int *)&inst->matrix.m[2][0];
		garageTop->matrix.m[2][2] = inst->matrix.m[2][2];
		garageTop->matrix.t[0] = inst->matrix.t[0];
		garageTop->matrix.t[1] = inst->matrix.t[1];
		garageTop->matrix.t[2] = inst->matrix.t[2];

		ratio = MATH_Sin((int)inst->instDef->rot.y);

		// continue setting GarageTop position
		garageTop->matrix.t[0] = inst->matrix.t[0] + (ratio * 0x4c >> 0xc);
		garageTop->matrix.t[1] = inst->matrix.t[1] + 0x300;

		ratio = MATH_Cos((int)inst->instDef->rot.y);

		// continue setting GarageTop position
		garageTop->matrix.t[2] = inst->matrix.t[2] + (ratio * 0x4c >> 0xc);

		garageTop->depthBiasNormal = 0xfe;

		garage->garageTopInst = garageTop;
	}

	if (levelID == GEM_STONE_VALLEY)
	{
		// ripper roo boss key
		bitIndex = ADV_REWARD_FIRST_BOSS_KEY;
		// check four boss keys
		for (i = 0; i < 4; i++)
		{
			if (CHECK_ADV_BIT(adv->rewards, bitIndex) == 0)
				goto GarageLocked;
			bitIndex++;
		}
		bossIsOpen = true;
	}

	// if not gemstone valley
	else
	{
		check = &data.advHubTrackIDs[(levelID - N_SANITY_BEACH) * 4];
		// check all four tracks on hub
		for (i = 0; i < 4; i++)
		{
			// if any trophy on this hub is not unlocked
			if (CHECK_ADV_BIT(adv->rewards, check[i] + ADV_REWARD_FIRST_TROPHY) == 0)
				// boss is not open
				goto GarageLocked;
		}
	}

	// if boss is open
	if (bossIsOpen)
	{
		// check if boss was NOT beaten on this hub (levelID - 0x19)
		bitIndex = data.BeatBossPrize[levelID - GEM_STONE_VALLEY];
		t->modelIndex = (CHECK_ADV_BIT(adv->rewards, bitIndex) != 0) ? 2 : 1;
	}

	// if boss is not open
	else
	{
	GarageLocked:
		// locked
		t->modelIndex = 0;
	}

	garage->rot.x = inst->instDef->rot.x;
	garage->rot.y = inst->instDef->rot.y;
	garage->rot.z = inst->instDef->rot.z;

	inst->depthBiasNormal = 1;
	inst->depthBiasSecondary = inst->depthBiasNormal;
	inst->unk53 = 0;
	inst->vertSplit = inst->instDef->pos.y + 0x300;
}
