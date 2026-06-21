#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3144-0x800b344c.
void AH_Pause_Update()
{
	struct GameTracker *gGT;
	gGT = sdata->gGT;

	if (D232.ptrPauseObject == 0)
	{
		struct PauseObject *ptrPauseObject;
		ptrPauseObject = &D232.pauseObject;

		D232.pausePageTimer = 0;
		D232.pausePageCurr = gGT->levelID - GEM_STONE_VALLEY;
		gGT->advPausePage = D232.pausePageCurr;

		// 0 = size
		// 0 = no relation to param4
		// 0x300 = SmallStackPool
		// 0xd = "other" thread bucket
		struct Thread *t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(0, NONE, SMALL, OTHER), 0, R232.s_PAUSE, 0);

		D232.ptrPauseObject = ptrPauseObject;
		ptrPauseObject->t = t;

		for (int i = 0; i < 0xe; i++)
		{
			struct Instance *inst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_GEM], R232.s_pause, t);

			ptrPauseObject->PauseMember[i].indexAdvPauseInst = -1;
			ptrPauseObject->PauseMember[i].inst = inst;
			ptrPauseObject->PauseMember[i].rot.x = 0;
			ptrPauseObject->PauseMember[i].rot.y = 0;
			ptrPauseObject->PauseMember[i].rot.z = 0;

			inst->flags |= (USE_SPECULAR_LIGHT | SCREENSPACE_INSTANCE | HIDE_MODEL);

			struct InstDrawPerPlayer *idpp = INST_GETIDPP(inst);

			idpp[0].pushBuffer = &gGT->pushBuffer_UI;
			for (int j = 1; j < gGT->numPlyrCurrGame; j++)
				idpp[j].pushBuffer = 0;

			*(int *)&inst->matrix.m[0][0] = 0x1000;
			*(int *)&inst->matrix.m[0][2] = 0;
			*(int *)&inst->matrix.m[1][1] = 0x1000;
			*(int *)&inst->matrix.m[2][0] = 0;
			inst->matrix.m[2][2] = 0x1000;
			inst->matrix.t[2] = 0x100;
		}
	}

	int tap = sdata->buttonTapPerPlayer[0];

	if ((tap & (BTN_RIGHT | BTN_LEFT)) != 0)
	{
		if ((tap & BTN_LEFT) != 0)
		{
			D232.pausePageDir = -1;
			gGT->advPausePage += -1;

			if (gGT->advPausePage < 0)
				gGT->advPausePage = 6;
		}

		// assume BTN_RIGHT
		else
		{
			D232.pausePageDir = 1;
			gGT->advPausePage += 1;

			if (gGT->advPausePage > 6)
				gGT->advPausePage = 0;
		}

		// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3340-0x800b3350 for adventure pause page-turn SFX.
		OtherFX_Play(0, 1);
	}

	// page is flipping
	if (D232.pausePageTimer > 0)
		D232.pausePageTimer--;

	// page is not flipping, flip desired
	else if (gGT->advPausePage != D232.pausePageCurr)
	{
		D232.pausePagePrev = D232.pausePageCurr;
		D232.pausePageDir_dup = D232.pausePageDir;

		D232.pausePageTimer = 8;

		D232.pausePageCurr = gGT->advPausePage;
	}

	int pageID;
	int posX;

	// second half
	if (D232.pausePageTimer < 5)
	{
		pageID = D232.pausePageCurr;
		posX = D232.pausePageTimer * D232.pausePageDir * -0x80;
	}

	// first half
	else
	{
		pageID = D232.pausePagePrev;
		posX = (8 - D232.pausePageTimer) * D232.pausePageDir * 0x80;
	}

	AH_Pause_Draw(pageID, posX);
}
