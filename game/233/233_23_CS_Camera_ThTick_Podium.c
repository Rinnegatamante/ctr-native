#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800aedf8-0x800af328
void CS_Camera_ThTick_Podium(struct Thread *th)
{
	struct GameTracker *gGT = sdata->gGT;
	u16 *podium = th->object;

	if (podium[0] == 0)
		gGT->drivers[0]->funcPtrs[0] = VehStuckProc_RIP_Init;

	if (gGT->cameraDC[0].cameraMode != 3)
	{
		if (OVR_233.cutsceneState < 1)
			OVR_233.cutsceneState = 1;

		OVR_233.PodiumInitUnk3 = 1;
	}

	if (((OVR_233.cutsceneState != 0 || OVR_233.boolStartToSkip != 0) && ((gGT->gameMode2 & CUP_NEW_WIN) != 0)) && sdata->ptrActiveMenu == NULL)
	{
		s16 stringIndex = 0x236;

		if ((gGT->gameMode2 & CUP_NEW_BATTLE) != 0)
			stringIndex = 0x237;

		TakeCupProgress_Activate(stringIndex);
		gGT->gameMode2 &= ~(CUP_NEW_WIN | CUP_NEW_BATTLE);
	}

	if (OVR_233.cutsceneState == 0 || sdata->ptrActiveMenu != NULL)
	{
		int numPoints = CAM_Path_GetNumPoints();
		int maxFrame = (numPoints << 0x15) >> 0x10;

		if (maxFrame != 0)
		{
			u16 frameTime = podium[0] + gGT->elapsedTimeMS;
			int frameTimeSigned = (s16)frameTime;
			s16 pos[3];
			s16 rot[3];
			s16 camPath[4];
			int frame;

			if (maxFrame - 0x12c0 < frameTimeSigned)
				OVR_233.PodiumInitUnk3 = 1;

			if (maxFrame <= frameTimeSigned)
			{
				frameTime = numPoints * 0x20 - 1;

				if (OVR_233.cutsceneState < 1)
					OVR_233.cutsceneState = 1;
			}

			frame = ((int)frameTime << 16) >> 21;
			OVR_233.PodiumInitUnk2 = frame;
			podium[0] = frameTime;

			CAM_Path_Move(frame, pos, rot, camPath);

			gGT->pushBuffer[0].pos[0] = pos[0];
			gGT->pushBuffer[0].pos[1] = pos[1];
			gGT->pushBuffer[0].pos[2] = pos[2];
			gGT->pushBuffer[0].rot[0] = rot[0];
			gGT->pushBuffer[0].rot[1] = rot[1];
			gGT->pushBuffer[0].rot[2] = rot[2];
		}
	}
	else
	{
		if ((gGT->gameMode2 & CUP_NEW_WIN) != 0)
			goto check_skip_button;

		DECOMP_DecalFont_DrawLine(sdata->lngStrings[0xc9], 0x100, 0xbe, FONT_BIG, JUSTIFY_CENTER | ORANGE);
	}

	if (((gGT->gameMode2 & CUP_NEW_WIN) == 0) && sdata->ptrActiveMenu == NULL)
	{
		u32 tapped = sdata->gGamepads->gamepad[0].buttonsTapped;
		s16 rewardId;

		if (((tapped & BTN_START) == 0) && ((OVR_233.cutsceneState == 0 || (tapped & (BTN_START | BTN_CROSS_one)) == 0)) &&
		    ((gGT->gameMode2 & VEH_FREEZE_PODIUM) != 0))
		{
			return;
		}

		if ((gGT->gameMode1 & ADVENTURE_MODE) == 0)
		{
			sdata->mainMenuState = 0;
			gGT->gameMode1 = (gGT->gameMode1 & ~ADVENTURE_ARENA) | MAIN_MENU;
			gGT->podiumRewardID = NOFUNC;
			gGT->gameMode2 &= ~VEH_FREEZE_PODIUM;

			RaceFlag_SetDrawOrder(0);
			MainRaceTrack_RequestLoad(MAIN_MENU_LEVEL);
			return;
		}

		OVR_233.PodiumInitUnk3 = 1;
		rewardId = gGT->podiumRewardID;
		gGT->numWinners = 0;
		gGT->renderFlags &= ~4;

		if (rewardId != STATIC_BIG1)
		{
			if (DECOMP_CS_Camera_BoolGotoBoss() == 0)
			{
				s16 hintID;

				OVR_233.isCutsceneOver = 1;
				th->flags |= 0x800;

				DECOMP_CS_DestroyPodium_StartDriving();

				switch (rewardId)
				{
				case STATIC_TROPHY:
					hintID = 0xc;
					break;
				case STATIC_RELIC:
					hintID = 0x13;
					break;
				case STATIC_KEY:
					hintID = 0xd;
					break;
				case STATIC_TOKEN:
					hintID = 0x14;
					break;
				default:
					hintID = 0x15;
					break;
				}

				if ((VehPickupItem_MaskBoolGoodGuy(gGT->drivers[0]) & 0xffff) == 0)
					hintID += 0x1f;

				CDSYS_XAPauseForce();
				CDSYS_XAPlay(1, hintID);

				gGT->podiumRewardID = NOFUNC;
				return;
			}

			th->funcThTick = CS_Camera_ThTick_Boss;

			if (gGT->podiumRewardID != STATIC_RELIC)
			{
				OVR_233.bossCutsceneIndex = -1;
				return;
			}

			if (gGT->currAdvProfile.numRelics < 18)
			{
				OVR_233.bossCutsceneIndex = -1;
				return;
			}

			OVR_233.bossCutsceneIndex = gGT->levelID - GEM_STONE_VALLEY;
			return;
		}

		gGT->podiumRewardID = NOFUNC;
		gGT->gameMode1 &= ~ADVENTURE_ARENA;
		gGT->gameMode2 &= ~VEH_FREEZE_PODIUM;

		MainRaceTrack_RequestLoad((sdata->advProgress.rewards[2] & 0x100000) ? OXIDE_TRUE_ENDING : OXIDE_ENDING);
		return;
	}

check_skip_button:
	if ((sdata->gGamepads->gamepad[0].buttonsTapped & BTN_START) != 0)
		OVR_233.boolStartToSkip = 1;
}
