#include <common.h>

#ifndef REBUILD_PS1
void DECOMP_RB_Spider_DrawWebs(struct Thread *t, struct PushBuffer *pb)
{
	typedef struct
	{
		u32 tag;
		u32 tpage;
		LINE_F2 f2;
	} multiCmdPacket;

	struct GameTracker *gGT;
	struct PrimMem *primMem;
	MATRIX *m;
	multiCmdPacket *p;

	s16 sVar1;
	u16 uVar2;
	int iVar3;
	u32 lineColor;
	u32 *ot;
	int depth;
	u32 *puVar8;
	s16 *scratchpad;
	int iVar11;
	int numSpiders;
	int iVar13;
	int iVar16;
	u32 uVar17;

	gGT = sdata->gGT;
	primMem = &gGT->backBuffer->primMem;

	// quit if there are no spiders
	if (t == NULL)
		return;

	scratchpad = (s16 *)0x1f800000;

	// all threads
	for (numSpiders = 0; t != NULL; numSpiders++)
	{
		struct Instance *inst = t->inst;
		struct InstDef *instDef = inst->instDef;

		uVar2 = instDef->pos[0];
		sVar1 = instDef->pos[2];

		scratchpad[0] = uVar2;
		scratchpad[1] = instDef->pos[1] + 0x540;
		scratchpad[2] = sVar1;
		scratchpad[3] = 0;

		scratchpad[4] = uVar2;
		scratchpad[5] = t->inst->matrix.t[1] + 0x60;
		scratchpad[6] = sVar1;
		scratchpad[7] = 0;

		scratchpad += 8;

		t = t->siblingThread;
	}

	int i, j;
	int numPlyr;
	numPlyr = gGT->numPlyrCurrGame;

	p = primMem->curr;
	p = p + (numSpiders * numPlyr);
	if ((u32)p >= (u32)primMem->endMin100) // these casts may need to be (int) instead of (u32)
		return;

	// loop through all players
	for (i = 0; i < numPlyr; i++)
	{
		pb = &gGT->pushBuffer[i];
		m = &pb->matrix_ViewProj;

		// store on GTE
		gte_SetRotMatrix(m);
		gte_SetTransMatrix(m);

		scratchpad = (s16 *)0x1f800000;

		// 0x10 * numSpiders
		s16 *output = (s16 *)0x1F800050;

		// loop through spiders
		for (j = 0; j < numSpiders; j++)
		{
			// optimal code, but invalid depth,
			// makes the web look darker
#if 0
			// depth of each vertex
			//__asm__ ("swc2 $17, 0( %0 );" : : "r"(ptr)); SZ1
			//__asm__ ("swc2 $18, 0( %0 );" : : "r"(ptr)); SZ2
			//__asm__ ("swc2 $19, 0( %0 );" : : "r"(ptr)); SZ3
			
			// Need this in Level/Model code,
			// rtps is "single", rtpt is "triple"
            gte_ldv01(&scratchpad[0], &scratchpad[4]);
            gte_rtpt();
            gte_stsxy01(&output[0], &output[2]);
			
			// depth of 2nd vertex
			__asm__ ("swc2 $18, 0( %0 );" : : "r"(&depth));
#else

			gte_ldv0(&scratchpad[0]);
			gte_rtps();
			gte_stsxy(&output[0]);

			gte_ldv0(&scratchpad[4]);
			gte_rtps();
			gte_stsxy(&output[2]);

			// rtps (single) writes depth to stsz,
			// no need for averaging with avsz3 or stotz
			gte_stsz(&depth);
#endif
			scratchpad += 8;


			// if line is close enough to the screen
			// to be seen, then generate primitives
			if ((u32)(depth - 1) < (0x1200 - 1))
			{
				p = primMem->curr;
				primMem->curr = p + 1;

				p->tpage = 0xe1000a20;
				p->f2.tag = 0;

				*(int *)&p->f2.x0 = *(int *)&output[0];
				*(int *)&p->f2.x1 = *(int *)&output[2];

				lineColor = 0x3f;
				if (depth > 0xa00)
				{
					lineColor = ((0x1200 - depth) * 0x3f) >> 0xb;
				}

				p->f2.r0 = lineColor;
				p->f2.g0 = lineColor;
				p->f2.b0 = lineColor;
				p->f2.code = 0x42;

				depth = depth >> 6;
				if (depth > 0x3ff)
					depth = 0x3ff;

				// pushBuffer 0xf4, ptrOT
				ot = (u32 *)&pb->ptrOT[depth];

				// prim header, OT and prim len
				*(int *)p = *ot | 0x5000000;
				*ot = (u32)p & 0xffffff;
			}
		}
	}
}
#endif

s16 spiderArr[] = {
    // first 13
    0x4c0, 0x439, 0x3A6, 0x306, 0x266, 0x1c8, 0x139, 0xb9, 0x59, 0x17, 0, 0, 0,

    // next 13
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b9848-0x800b9bc0.

void DECOMP_RB_Spider_ThTick(struct Thread *t)
{
	s16 sVar2;
	int iVar3;

	char prevKartState;
	struct GameTracker *gGT;
	struct Instance *hitInst;
	struct Driver *victim;
	struct Instance *spiderInst;
	struct Spider *spider;

	spider = t->object;
	spiderInst = t->inst;

	if (spider->delay != 0)
	{
		spider->delay--;
		return;
	}

	spider->unused++;

	// If spider is on ground
	if (spider->boolNearRoof == 0)
	{
		if (4 < spider->animLoopCount)
		{
			// Play animation backwards
			sVar2 = spiderInst->animFrame - 1;

			// if animation is at beginning
			if (sVar2 < 1)
			{
				spiderInst->animFrame = 0;
				spider->animLoopCount = 0;
				spider->boolNearRoof = 1;
				goto setWiggleAnimation;
			}

			spiderInst->animFrame = sVar2;

			// last frame of last animation
			if (sVar2 == 0xc)
			{
				PlaySound3D(0x79, spiderInst);
			}

			goto updatePosScale;
		}

		sVar2 = spiderInst->animFrame;
		iVar3 = DECOMP_INSTANCE_GetNumAnimFrames(spiderInst, spiderInst->animIndex);

		if (iVar3 <= sVar2 + 1)
		{
			spiderInst->animFrame = 0;
			sVar2 = spider->animLoopCount;
			spider->animLoopCount = sVar2 + 1;

			if ((s16)(sVar2 + 1) == 5)
			{
				spiderInst->animIndex = 0;
				spiderInst->animFrame = DECOMP_INSTANCE_GetNumAnimFrames(spiderInst, 0) - 1;
			}

			goto checkCollision;
		}
	}

	// if spider is near ceiling
	else
	{
		if (4 < spider->animLoopCount)
		{
			sVar2 = spiderInst->animFrame;
			iVar3 = DECOMP_INSTANCE_GetNumAnimFrames(spiderInst, 0);

			if (sVar2 + 1 < iVar3)
			{
				spiderInst->animFrame++;
			}
			else
			{
				spider->animLoopCount = 0;
				spider->boolNearRoof = 0;
			setWiggleAnimation:
				spiderInst->animIndex = 1;
			}

		updatePosScale:
			spiderInst->matrix.t[1] = (int)spiderInst->instDef->pos[1] + spiderArr[spiderInst->animFrame];

			if (spiderInst->animFrame < 0xb)
			{
				spider->shadowInst->scale[0] = (s16)((spiderInst->animFrame << 0xc) / 10) + 0x1800;
				spider->shadowInst->scale[2] = (s16)((spiderInst->animFrame << 0xc) / 10) + 0x1800;
			}

			goto checkCollision;
		}

		sVar2 = spiderInst->animFrame;
		iVar3 = DECOMP_INSTANCE_GetNumAnimFrames(spiderInst, spiderInst->animIndex);

		if (iVar3 <= sVar2 + 1)
		{
			spiderInst->animFrame = 0;
			sVar2 = spider->animLoopCount;
			spider->animLoopCount = sVar2 + 1;

			if ((s16)(sVar2 + 1) == 5)
			{
				spiderInst->animIndex = 0;
				spiderInst->animFrame = 0;
				PlaySound3D(0x7a, spiderInst);
			}

			goto checkCollision;
		}
	}

	spiderInst->animFrame++;

checkCollision:
	gGT = sdata->gGT;

	hitInst = (struct Instance *)LinkedCollide_Radius(spiderInst, t, gGT->threadBuckets[PLAYER].thread, 0x9000);
	if (hitInst == NULL)
	{
		hitInst = (struct Instance *)LinkedCollide_Radius(spiderInst, t, gGT->threadBuckets[ROBOT].thread, 0x9000);
		if (hitInst == NULL)
		{
			hitInst = (struct Instance *)LinkedCollide_Radius(spiderInst, t, gGT->threadBuckets[MINE].thread, 0x9000);
			if (hitInst != NULL)
			{
				hitInst->thread->funcThCollide(hitInst->thread);
			}

			return;
		}

		victim = (struct Driver *)hitInst->thread->object;
		DECOMP_RB_Hazard_HurtDriver(victim, 1, 0, 0);
		return;
	}

	victim = (struct Driver *)hitInst->thread->object;
	prevKartState = victim->kartState;
	if ((DECOMP_RB_Hazard_HurtDriver(victim, 1, 0, 0) != 0) && (prevKartState != KS_SPINNING))
	{
		OtherFX_Play(0x7b, 1);
		Voiceline_RequestPlay(1, data.characterIDs[victim->driverID], 0x10);
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b9bc0-0x800b9bd4.

int DECOMP_RB_Spider_ThCollide(struct Thread *spiderThread, struct Thread *driverTh, void *funcThCollide, struct ScratchpadStruct *sps)
{
	(void)spiderThread;
	(void)driverTh;
	(void)funcThCollide;

	return (s16)sps->Input1.modelID == DYNAMIC_PLAYER;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b9bd4-0x800b9dd8.

void DECOMP_RB_Spider_LInB(struct Instance *inst)
{
	struct Spider *spider;
	s16 rot[3];
	struct Thread *t;
	struct Instance *shadowInst;
	int spiderID;

	if (inst->thread != NULL)
		return;

	t = DECOMP_PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Spider), NONE, SMALL, SPIDER), DECOMP_RB_Spider_ThTick, "spider", 0);
	inst->thread = t;
	if (t == NULL)
		return;

	spider = t->object;
	t->funcThCollide = (void (*)(struct Thread *))DECOMP_RB_Spider_ThCollide;
	t->inst = inst;

	inst->scale[0] = 0x1c00;
	inst->scale[1] = 0x1c00;
	inst->scale[2] = 0x1c00;
	inst->animIndex = 1;

	spiderID = inst->name[strlen(inst->name) - 1] - '0';
	spider->spiderID = spiderID;
	spider->boolNearRoof = 1;
	spider->animLoopCount = 0;

	if (spiderID == 3)
	{
		spider->delay = 91;
	}
	else if (spiderID == 2)
	{
		spider->delay = 69;
	}
	else
	{
		spider->delay = 0;
	}

	shadowInst = DECOMP_INSTANCE_Birth3D(sdata->gGT->modelPtr[DYNAMIC_SPIDERSHADOW], 0, t);

	spider->shadowInst = shadowInst;

	*(int *)&shadowInst->matrix.m[0][0] = *(int *)&inst->matrix.m[0][0];
	*(int *)&shadowInst->matrix.m[0][2] = *(int *)&inst->matrix.m[0][2];
	*(int *)&shadowInst->matrix.m[1][1] = *(int *)&inst->matrix.m[1][1];
	*(int *)&shadowInst->matrix.m[2][0] = *(int *)&inst->matrix.m[2][0];
	shadowInst->matrix.m[2][2] = inst->matrix.m[2][2];

	shadowInst->matrix.t[0] = inst->matrix.t[0];
	shadowInst->matrix.t[1] = inst->matrix.t[1] - 8;
	shadowInst->matrix.t[2] = inst->matrix.t[2];
	inst->matrix.t[1] += 0x4c0;

	shadowInst->scale[0] = 0x2000;
	shadowInst->scale[1] = 0x2000;
	shadowInst->scale[2] = 0x2000;

	rot[0] = 0;
	rot[1] = 0x200;
	rot[2] = 0;
	ConvertRotToMatrix(&shadowInst->matrix, &rot[0]);
}
