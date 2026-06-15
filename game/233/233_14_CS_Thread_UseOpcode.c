#include <common.h>

static void CS_SaveDecodedOpcode(const struct CutsceneObj *cs, int out[5])
{
	const int *src = (const int *)&cs->decodedOpcode;
	out[0] = src[0];
	out[1] = src[1];
	out[2] = src[2];
	out[3] = src[3];
	out[4] = src[4];
}

static void CS_RestoreDecodedOpcode(struct CutsceneObj *cs, const int in[5])
{
	int *dst = (int *)&cs->decodedOpcode;
	dst[0] = in[0];
	dst[1] = in[1];
	dst[2] = in[2];
	dst[3] = in[3];
	dst[4] = in[4];
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac840-0x800ade8c
int CS_Thread_UseOpcode(struct Instance *instance, struct CutsceneObj *cs)
{
	u8 numPlayers;
	int frameBoundaryHit;
	s16 numCamPathPoints;
	s16 gameModeTarget;
	u16 clockEffectFlags;
	u16 cutsceneFlags;
	u32 conditionMet;
	int iVar8;
	char *const *cutsceneOpcodes;
	int iVar10;
	s16 levelToLoad;
	int distanceToScreen;
	struct Thread *dancerThread;
	char *opcodeAt;
	int iVar12;
	struct CsOpcodeMeta *opcodeMeta;
	s16 *opcodeMetaShorts;
	s16 *frameData;
	int rotInterpNumerator;
	int rotInterpStartFrame;
	int rotInterpFrameRange;
	int nextFrameTime;
	int lodIndex;
	struct ModelHeader *modelHeader;
	int metadataBackup[5];
	s16 camRot[3];
	s16 camPos[3];
	u16 camPathFlags[2];
	int animIndex;
	int opcodeDuration;
	int opcodeChanged;
	int elapsedTimeRemaining;

	struct GameTracker *gGT = sdata->gGT;
	CS_SaveDecodedOpcode(cs, metadataBackup);

	if (instance != 0)
	{
		if ((instance->flags & SPLIT_LINE) != 0)
			instance->vertSplit = D233.VertSplitLine;

		if ((int)instance->model->id == (u32)(u8)gGT->podium_modelIndex_Second)
		{
			if (D233.PodiumInitUnk2 - 0x65U < 0x87)
			{
				instance->flags |= HIDE_MODEL;
			}
			else
			{
				if ((instance->flags & HIDE_MODEL) == 0)
					goto afterPodiumSecondModelCheck;
				instance->depthBiasNormal -= 2;
				instance->depthBiasSecondary -= 2;
				instance->flags &= ~HIDE_MODEL;
			}
		}
	afterPodiumSecondModelCheck:

		if ((int)instance->model->id == (u32)(u8)gGT->podium_modelIndex_First)
		{
			if (D233.PodiumInitUnk2 - 0x83U < 0x69)
			{
				instance->flags |= HIDE_MODEL;
			}
			else
			{
				if ((instance->flags & HIDE_MODEL) == 0)
					goto afterPodiumFirstModelCheck;
				instance->depthBiasNormal -= 6;
				instance->depthBiasSecondary -= 6;
				instance->flags &= ~HIDE_MODEL;
			}
		}
	afterPodiumFirstModelCheck:

		if ((cs->flags & 0x80) != 0)
		{
			if (((int)instance->model->id + -0xce == (int)gGarage.unusedArr_garageChars[sdata->advCharSelectIndex_curr]) && (gGarage.boolSelected == 1))
			{
				if ((cs->flags & 0x100) == 0)
				{
					gGT->pushBuffer[0].fadeFromBlack_currentValue = 0x1fff;
					gGT->pushBuffer[0].fadeFromBlack_desiredResult = 0x1000;
					gGT->pushBuffer[0].fade_step = 0xfd56;
					cs->flags |= 0x100;
					CS_ScriptCmd_OpcodeAt(cs, R233.advCharSelectSelectOpcodes[(int)instance->model->id - STATIC_CRASHSELECT]);
					CS_SaveDecodedOpcode(cs, metadataBackup);
				reloadAdvCharSelectOpcodeState:
					cs->unk18 = ((int *)&cs->decodedOpcode)[2];
					iVar8 = MixRNG_Scramble();
					opcodeMeta = (struct CsOpcodeMeta *)cs->metadata;
					opcodeMetaShorts = (s16 *)opcodeMeta;
					cs->opcodeDuration =
					    opcodeMeta->frameStart + (s16)((int)((iVar8 >> 2 & 0xfff) * (((int)opcodeMeta->frameEnd - (int)opcodeMeta->frameStart) + 1)) >> 0xc);
				}
			}
			else
			{
				if ((cs->flags & 0x100) != 0)
				{
					cs->flags &= 0xfeff;
					CS_ScriptCmd_OpcodeAt(cs, R233.advCharSelectDeselectOpcodes[(int)instance->model->id - STATIC_CRASHSELECT]);
					CS_SaveDecodedOpcode(cs, metadataBackup);
					goto reloadAdvCharSelectOpcodeState;
				}
			}
		}
	}

	opcodeDuration = (int)cs->opcodeDuration;
	iVar12 = cs->unk18;
	iVar8 = (int)cs->unk1e;
	elapsedTimeRemaining = gGT->elapsedTimeMS;
	opcodeMeta = (struct CsOpcodeMeta *)cs->metadata;
	opcodeMetaShorts = (s16 *)opcodeMeta;
	animIndex = (int)opcodeMeta->animIndex;

	if (instance == 0)
	{
		numCamPathPoints = CAM_Path_GetNumPoints();
		if ((int)numCamPathPoints != 0)
		{
			iVar10 = ((s32)((u32)gGT->msInThisLEV << 11)) >> 16;
			if (iVar10 < (int)numCamPathPoints + -1)
			{
				CAM_Path_Move(iVar10, camPos, camRot, camPathFlags);
				gGT->pushBuffer[0].pos[0] = camPos[0];
				gGT->pushBuffer[0].pos[1] = camPos[1];
				gGT->pushBuffer[0].pos[2] = camPos[2];
				gGT->pushBuffer[0].rot[0] = camRot[0];
				gGT->pushBuffer[0].rot[1] = camRot[1];
				gGT->pushBuffer[0].rot[2] = camRot[2];
			}
			else
			{
				if (opcodeMeta->opcode == 0x14)
					CS_ScriptCmd_OpcodeNext(cs);
				CAM_Path_Move((int)(s16)(numCamPathPoints + -1), gGT->pushBuffer[0].pos, gGT->pushBuffer[0].rot, camPathFlags);
			}

			clockEffectFlags = gGT->clockEffectEnabled;
			gGT->clockEffectEnabled = clockEffectFlags & 0xfffe;
			if ((camPathFlags[0] & 1) != 0)
				gGT->clockEffectEnabled = clockEffectFlags & 0xfffe | 1;

			if ((cs->flags & 0x20) == 0)
			{
				gGT->pushBuffer[0].distanceToScreen_PREV = 0x100;
				if ((camPathFlags[0] & 2) != 0)
					gGT->pushBuffer[0].distanceToScreen_PREV = 0x50;
				if ((camPathFlags[0] & 4) != 0)
					gGT->pushBuffer[0].distanceToScreen_PREV = 0x278;
				if ((camPathFlags[0] & 0x20) != 0)
					gGT->pushBuffer[0].distanceToScreen_PREV = 0x1eb;
				if ((camPathFlags[0] & 0x40) != 0)
					gGT->pushBuffer[0].distanceToScreen_PREV = 0x14d;
			}

			if (((camPathFlags[0] & 0x10) != 0) && ((MixRNG_Scramble() & 0xf) == 0))
			{
				CTR_Box_DrawClearBox(&R233.introClearBoxRect, &R233.introClearBoxColor, 1, gGT->backBuffer->otMem.uiOT);
			}

			if (gGT->levelID == 0x29)
				gGT->pushBuffer[0].distanceToScreen_PREV = 0x140;

			gGT->pushBuffer[0].distanceToScreen_CURR = gGT->pushBuffer[0].distanceToScreen_PREV;
		}

		if ((sdata->gGamepads->gamepad[0].buttonsTapped & BTN_START) != 0)
		{
			gGT->clockEffectEnabled &= 0xfffe;
			if (0x13 < gGT->levelID - 0x2cU)
			{
				if (gGT->levelID == 0x29)
				{
					if ((u32)gGT->msInThisLEV >> 5 < 0xb5)
						goto afterCameraAndSkipChecks;
					RaceFlag_SetCanDraw(1);
					iVar8 = RaceFlag_IsTransitioning();
					if ((iVar8 == 0) && (iVar8 = RaceFlag_IsFullyOnScreen(), iVar8 == 0))
						RaceFlag_SetFullyOffScreen();
				}
				else
				{
					RaceFlag_SetCanDraw(1);
					iVar8 = RaceFlag_IsTransitioning();
					if ((iVar8 == 0) && (iVar8 = RaceFlag_IsFullyOnScreen(), iVar8 == 0))
						RaceFlag_SetFullyOffScreen();
					levelToLoad = CREDITS_CRASH;
					if (gGT->levelID - 0x2aU < 2)
						goto requestSkipLevelLoad;
				}
				CseqMusic_StopAll();
				CDSYS_XAPauseRequest();
				RaceFlag_SetDrawOrder(0);
				levelToLoad = MAIN_MENU_LEVEL;
			requestSkipLevelLoad:
				MainRaceTrack_RequestLoad(levelToLoad);
				D233.isCutsceneOver = 1;
				gGT->gameMode2 &= ~VEH_FREEZE_PODIUM;
				CS_RestoreDecodedOpcode(cs, metadataBackup);
				return 1;
			}
			CS_Credits_End();
		}
	}

afterCameraAndSkipChecks:
	opcodeChanged = 0;
	if (elapsedTimeRemaining == 0)
	{
	updateInstanceAndReturn:
		cs->unk18 = iVar12;
		cs->animIndex = (char)animIndex;
		cs->unk1e = (s16)iVar8;
		cs->opcodeDuration = (s16)opcodeDuration;
		iVar10 = (int)opcodeMeta->rotStart;
		iVar12 = iVar12 >> 5;
		if (iVar10 != (int)opcodeMeta->rotEnd)
		{
			rotInterpStartFrame = opcodeMeta->arg0.i;
			if (opcodeMeta->arg1.i != rotInterpStartFrame)
			{
				rotInterpNumerator = ((((int)opcodeMeta->rotEnd - iVar10) + 0x800U & 0xfff) - 0x800) * (iVar12 - rotInterpStartFrame);
				rotInterpFrameRange = opcodeMeta->arg1.i - rotInterpStartFrame;
				if (rotInterpFrameRange < 0)
					rotInterpFrameRange = -rotInterpFrameRange;
				iVar10 = iVar10 + rotInterpNumerator / rotInterpFrameRange;
			}
		}
		iVar10 = iVar10 + cs->unk1c;
		if ((iVar10 != (int)cs->unk22) && (cs->unk22 = (s16)iVar10, instance != 0))
		{
			ConvertRotToMatrix(&instance->matrix, &cs->unk20);
		}
		iVar8 = CS_Instance_SafeCheckAnimFrame(instance, animIndex, iVar8, iVar12);
		if (iVar12 != iVar8)
		{
			animIndex &= ~0xff;
			iVar12 = 0;
		}
		if (instance != 0)
		{
			instance->animFrame = (s16)iVar12;
			instance->animIndex = (char)animIndex;
		}
		if (cs->frameOverrideRoot != 0)
		{
			frameData = (s16 *)((uintptr_t)(*cs->frameOverrideRoot) + iVar12 * 0x20);
			*(int *)((u8 *)&instance->matrix + 0x00) = *(int *)(frameData + 4);
			*(int *)((u8 *)&instance->matrix + 0x04) = *(int *)(frameData + 6);
			*(int *)((u8 *)&instance->matrix + 0x08) = *(int *)(frameData + 8);
			*(int *)((u8 *)&instance->matrix + 0x0c) = *(int *)(frameData + 10);
			*(int *)((u8 *)&instance->matrix + 0x10) = *(int *)(frameData + 0xc);
			instance->matrix.t[0] = (int)*frameData;
			instance->matrix.t[1] = (int)frameData[1];
			instance->matrix.t[2] = (int)frameData[2];
		}
		return 0;
	}

processOpcode:
	switch (opcodeMeta->opcode)
	{
	case 0:
	case 0x2a:
	case 0x2b:
		if (instance != 0)
		{
			cutsceneFlags = cs->flags;
			if ((cutsceneFlags & 0x200) != 0)
			{
				if (((cutsceneFlags & 0x400) == 0) && (sdata->XA_State == 3))
					cs->flags = cutsceneFlags | 0x400;
				if (sdata->XA_State != 0)
				{
					if ((cs->flags & 0x400) == 0)
						iVar12 = 0;
					else
					{
						iVar12 = CS_Instance_SafeCheckAnimFrame(instance, animIndex, iVar8, (sdata->XA_CurrOffset * 0x1e00) / 0xac44);
						iVar12 = iVar12 << 5;
					}
					if (opcodeMeta->arg1.i << 5 < iVar12)
						break;
					goto updateInstanceAndReturn;
				}
				break;
			}
		}
		if (opcodeChanged != 0)
		{
			animIndex = (int)opcodeMeta->animIndex;
			iVar12 = CS_Instance_SafeCheckAnimFrame(instance, animIndex, iVar8, opcodeMeta->arg0.i);
			iVar12 = iVar12 << 5;
			iVar10 = MixRNG_Scramble();
			opcodeChanged = 0;
			opcodeDuration =
			    ((int)((iVar10 >> 2 & 0xfff) * (((int)opcodeMeta->frameEnd - (int)opcodeMeta->frameStart) + 1)) >> 0xc) + (int)opcodeMeta->frameStart;
		}
		frameBoundaryHit = 0;
		if (opcodeMeta->arg1.i < opcodeMeta->arg0.i)
		{
			iVar10 = opcodeMeta->arg1.i * 0x20;
			iVar12 = iVar12 - elapsedTimeRemaining;
			if (iVar12 < iVar10)
			{
				elapsedTimeRemaining = iVar10 - iVar12;
			markAnimationBoundary:
				frameBoundaryHit = 1;
			}
		}
		else
		{
			iVar10 = CS_Instance_SafeCheckAnimFrame(instance, animIndex, iVar8, opcodeMeta->arg1.i);
			nextFrameTime = (iVar10 + 1) * 0x20;
			iVar12 = iVar12 + elapsedTimeRemaining;
			if (nextFrameTime <= iVar12)
			{
				frameBoundaryHit = 1;
				elapsedTimeRemaining = 0;
				if (nextFrameTime != 0)
				{
					elapsedTimeRemaining = iVar12 + (iVar10 + 1) * -0x20;
					goto markAnimationBoundary;
				}
			}
		}
		if ((frameBoundaryHit) || (opcodeDuration < 1))
		{
			opcodeDuration = opcodeDuration + -1;
			if (opcodeDuration < 1)
			{
				CS_ScriptCmd_OpcodeNext(cs);
				opcodeChanged = 1;
			}
			else
			{
				iVar12 = CS_Instance_SafeCheckAnimFrame(instance, animIndex, iVar8, opcodeMeta->arg0.i);
				iVar12 = iVar12 << 5;
			}
		}
		else
		{
			elapsedTimeRemaining = 0;
		}
		goto finishOpcodeStep;

	case 1:
		opcodeChanged = 1;
		CS_ScriptCmd_OpcodeAt(cs, opcodeMeta->arg1.ptr);
		goto finishOpcodeStep;

	case 2:
		if (instance != 0)
			instance->flags |= HIDE_MODEL;
		CS_RestoreDecodedOpcode(cs, metadataBackup);
		return 1;

	case 3:
		if (instance != 0)
		{
			// Retail builds this opcode 3 init data at scratchpad 0x1f800108.
			struct CsThreadInitData *initData = CTR_SCRATCHPAD_PTR(struct CsThreadInitData, 0x108);
			int spawnModelID = opcodeMeta->arg1.i;

			CS_Instance_GetFrameData(instance, (int)opcodeMeta->animIndex, opcodeMeta->arg0.i, (u16 *)initData->podiumPos, (u16 *)initData->rot, 0);

			initData->podiumPos[0] += (s16)instance->matrix.t[0];
			initData->podiumPos[1] += (s16)instance->matrix.t[1];
			initData->podiumPos[2] += (s16)instance->matrix.t[2];
			initData->characterPos[0] = 0;
			initData->characterPos[1] = 0;
			initData->characterPos[2] = 0;

			if (spawnModelID == NDI_BOX_PARTICLES_01)
			{
				initData->rot[0] = 0;
				initData->rot[1] = 0;
				initData->rot[2] = 0;
			}

			CS_Thread_Init(spawnModelID, R233.s_spawn, (s16 *)initData, 0, instance->thread);
		}
		break;

	case 4:
		iVar10 = MixRNG_Scramble();
		if (opcodeMeta->arg0.i < (int)(iVar10 >> 2 & 0xff))
			CS_ScriptCmd_OpcodeNext(cs);
		else
			CS_ScriptCmd_OpcodeAt(cs, opcodeMeta->arg1.ptr);
		opcodeChanged = 1;
		goto finishOpcodeStep;

	case 5:
		if (gGT->levelID == 0x28)
		{
			if (instance != 0)
				Garage_PlayFX(opcodeMeta->arg1.u, (int)instance->model->id + -0xce);
		}
		else
		{
			iVar10 = CS_Instance_BoolPlaySound(cs, instance);
			if (iVar10 != 0)
				OtherFX_Play((u32)(u16)opcodeMetaShorts[6], 1);
		}
		break;

	case 6:
		OtherFX_Stop2((u32)(u16)opcodeMetaShorts[6]);
		break;

	case 7:
		CseqMusic_Start((u32)(u16)opcodeMetaShorts[6], 0, 0, 0, opcodeMeta->arg0.i);
		break;

	case 8:
		CseqMusic_Restart((u32)(u16)opcodeMetaShorts[6], 1);
		break;

	case 9:
		if (instance != 0)
		{
			iVar10 = (int)instance->model->numHeaders;
			if ((iVar10 != 0) && (modelHeader = instance->model->headers, modelHeader != 0))
			{
				lodIndex = opcodeMeta->arg1.i;
				iVar8 = lodIndex;
				if (iVar10 <= lodIndex)
				{
					lodIndex = iVar10 + -1;
					iVar8 = lodIndex;
				}
				while (lodIndex != 0)
				{
					modelHeader->maxDistanceLOD = 0;
					lodIndex = lodIndex + -1;
					modelHeader++;
				}
				modelHeader->maxDistanceLOD = 20000;
			}
		}
		break;

	case 10:
		if (opcodeMeta->arg1.i == -1)
			cutsceneFlags = cs->flags | CS_FLAG_PATH_MOTION_DISABLED;
		else
		{
			cs->pathProgress32 = 0;
			cutsceneFlags = cs->flags & ~CS_FLAG_PATH_MOTION_DISABLED;
		}
		cs->flags = cutsceneFlags;
		break;

	case 0xb:
		cs->desiredScale = opcodeMetaShorts[4];
		cs->scaleSpeed = opcodeMetaShorts[6];
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case 0xc:
		gGT->pushBuffer[0].fadeFromBlack_currentValue = 0x1fff;
		gGT->pushBuffer[0].fadeFromBlack_desiredResult = 0x1000;
		gGT->pushBuffer[0].fade_step = 0xfd56;
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case 0xd:
		cutsceneFlags = cs->flags | opcodeMetaShorts[6];
		goto setFlagsAndAdvanceOpcode;

	case 0xe:
		cs->flags &= ~opcodeMetaShorts[6];
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case 0xf:
		gGT->bool_AdvHub_NeedToSwapLEV = 1;
		if ((gGT->gameMode2 & CREDITS) == 0)
		{
			cutsceneOpcodes = R233.introCutsceneOpcodes;
			iVar10 = gGT->levelID + -0x1e;
		}
		else
		{
			cutsceneOpcodes = R233.creditsCutsceneOpcodes;
			iVar10 = gGT->levelID + -0x2c;
		}
		CS_ScriptCmd_OpcodeAt(cs, cutsceneOpcodes[iVar10]);
		goto updateInstanceAndReturn;

	case 0x10:
		iVar10 = opcodeMeta->arg1.i;
		gGT->levelID = iVar10;
		if (iVar10 == 0x1e)
		{
			RaceFlag_SetCanDraw(0);
		requestDirectLevelLoad:
			gGT->gameMode2 &= ~VEH_FREEZE_PODIUM;
			MainRaceTrack_RequestLoad((int)(s16)iVar10);
		}
		else
		{
			if (iVar10 < 0x1f)
			{
				if (iVar10 == 0x19)
				{
					levelToLoad = GEM_STONE_VALLEY;
				requestMappedLevelLoad:
					gGT->gameMode2 &= ~VEH_FREEZE_PODIUM;
					MainRaceTrack_RequestLoad(levelToLoad);
					break;
				}
			}
			else
			{
				if (iVar10 == 0x27)
				{
					RaceFlag_SetDrawOrder(0);
					levelToLoad = MAIN_MENU_LEVEL;
					goto requestMappedLevelLoad;
				}
				if (iVar10 == 0x2c)
					goto requestDirectLevelLoad;
			}
			D233.boolLoadNextSwap = 1;
			LOAD_Hub_ReadFile(sdata->ptrBigfileCdPos_2, iVar10, 3 - (int)gGT->activeMempackIndex);
		}
		break;

	case 0x11:
		if ((D233.boolLoadNextSwap == 0) || (sdata->queueReady == 0) || (sdata->queueLength != 0))
			goto updateInstanceAndReturn;
		break;

	case 0x12:
		CDSYS_XAPlay(opcodeMeta->arg0.i, opcodeMeta->arg1.i);
		if (sdata->XA_State != 0)
			cs->flags = cs->flags & 0xfbff | 0x200;
		break;

	case 0x13:
		if (sdata->XA_State == 0)
		{
			cutsceneFlags = cs->flags & 0xfdff;
			goto setFlagsAndAdvanceOpcode;
		}

	case 0x14:
		goto updateInstanceAndReturn;

	case 0x15:
		numPlayers = gGT->numPlyrCurrGame;
		gGT->stars.numStars = (s16)((int)gGT->level1->stars.numStars / (int)(u32)numPlayers);
		gGT->stars.spread = gGT->level1->stars.spread;
		gGT->stars.seed = gGT->level1->stars.seed;
		gGT->stars.distance = gGT->level1->stars.distance;
		D233.boolLoadNextSwap = 0;
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case 0x16:
		iVar10 = RaceFlag_IsFullyOffScreen();
		if (iVar10 == 1)
		{
			RaceFlag_SetCanDraw(1);
			RaceFlag_BeginTransition(1);
		}
		break;

	case 0x17:
		conditionMet = RaceFlag_IsFullyOnScreen();
		goto advanceIfConditionMet;

	case 0x18:
		iVar10 = RaceFlag_IsFullyOnScreen();
		if (iVar10 == 1)
			RaceFlag_BeginTransition(2);
		break;

	case 0x19:
		cs->particleID = opcodeMetaShorts[6];
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case 0x1a:
		if (instance != 0)
			instance->flags |= HIDE_MODEL;
		break;

	case 0x1b:
		if (instance != 0)
			instance->flags &= ~HIDE_MODEL;
		break;

	case 0x1c:
		if (instance != 0)
		{
			instance->depthBiasNormal += (char)opcodeMeta->arg1.i;
			instance->depthBiasSecondary += (char)opcodeMeta->arg1.i;
		}
		break;

	case 0x1d:
		if (instance != 0)
			instance->flags |= opcodeMeta->arg1.u;
		break;

	case 0x1e:
		if (instance != 0)
			instance->flags &= ~opcodeMeta->arg1.u;
		break;

	case 0x1f:
		cs->unk4 = 0x1333;
		break;

	case 0x20:
		D233.isCutsceneOver = 1;
		CS_DestroyPodium_StartDriving();
		D233.bossCutsceneIndex = -1;
		gGT->overlayTransition = 3;
		gGT->gameMode2 &= ~VEH_FREEZE_PODIUM;
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case 0x21:
		D233.bossCutsceneIndex = opcodeMeta->arg1.i;
		if ((D233.bossCutsceneIndex == 0) && (0x11 < gGT->currAdvProfile.numRelics))
			D233.bossCutsceneIndex = 9;
		D233.cutsceneState = 1;
		break;

	case 0x22:
		distanceToScreen = opcodeMeta->arg1.i;
		gGT->pushBuffer[0].distanceToScreen_PREV = distanceToScreen;
		gGT->pushBuffer[0].distanceToScreen_CURR = distanceToScreen;
		cutsceneFlags = cs->flags | 0x20;
	setFlagsAndAdvanceOpcode:
		cs->flags = cutsceneFlags;
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case 0x23:
		conditionMet = CS_Credits_IsTextValid();
		goto advanceIfConditionMet;

	case 0x24:
	{
		// Retail builds this credits dancer init data at scratchpad 0x1f800108.
		struct CsThreadInitData *initData = CTR_SCRATCHPAD_PTR(struct CsThreadInitData, 0x108);
		int dancerModelID = opcodeMeta->arg1.i;

		initData->podiumPos[0] = 0;
		initData->podiumPos[1] = 0;
		initData->podiumPos[2] = 0;
		initData->rot[0] = 0;
		initData->rot[1] = 0;
		initData->rot[2] = 0;
		initData->characterPos[0] = 0;
		initData->characterPos[1] = 0;
		initData->characterPos[2] = 0;

		gGT->podium_modelIndex_First = (u8)dancerModelID;
		gGT->podium_modelIndex_Second = 0;
		gGT->podium_modelIndex_Third = 0;

		if (dancerModelID == STATIC_OXIDEDANCE)
		{
			gGT->podium_modelIndex_First = 0;
			gGT->podium_modelIndex_Second = STATIC_OXIDEDANCE;
		}
		if (dancerModelID == STATIC_CRASHDANCE)
			initData->rot[1] += 0x800;

		initData->rot[0] += R233.creditsDancerRotOffset[0];
		initData->rot[1] += R233.creditsDancerRotOffset[1];
		initData->rot[2] += R233.creditsDancerRotOffset[2];

		dancerThread = (struct Thread *)CS_Thread_Init(dancerModelID, R233.s_g_dancer, (s16 *)initData, 0, 0);
		CS_Credits_NewDancer(dancerThread, (int)opcodeMetaShorts[6]);
	}
	break;

	case 0x25:
		conditionMet = CS_Credits_NewCreditGhosts();
	advanceIfConditionMet:
		conditionMet &= 0xffff;
		if (conditionMet == 0)
			goto updateInstanceAndReturn;
		break;

	case 0x26:
		if (opcodeMeta->frameEnd == 0)
		{
			if ((opcodeMeta->arg0.i != (int)gGarage.unusedArr_garageChars[sdata->advCharSelectIndex_curr]) || (gGarage.boolSelected == 0))
			{
				opcodeAt = opcodeMeta->arg1.ptr;
			branchToGarageOpcode:
				opcodeChanged = 1;
				CS_ScriptCmd_OpcodeAt(cs, opcodeAt);
			}
		}
		else
		{
			if ((opcodeMeta->arg0.i == (int)gGarage.unusedArr_garageChars[sdata->advCharSelectIndex_curr]) && (gGarage.boolSelected == 1))
			{
				opcodeAt = opcodeMeta->arg1.ptr;
				goto branchToGarageOpcode;
			}
		}
		break;

	case 0x27:
		if ((u32)gGT->msInThisLEV >> 5 < opcodeMeta->arg1.u)
			goto updateInstanceAndReturn;
		break;

	case 0x28:
		iVar12 = CS_Instance_SafeCheckAnimFrame(instance, animIndex, iVar8, iVar12 >> 5);
		iVar12 = iVar12 << 5;
		goto updateInstanceAndReturn;

	case 0x29:
		CS_Credits_End();
		CS_RestoreDecodedOpcode(cs, metadataBackup);
		return 1;

	case 0x2c:
		gameModeTarget = opcodeMeta->animIndex;
		if (gameModeTarget == 1)
			gGT->gameMode2 |= opcodeMeta->arg1.u;
		else
		{
			if (gameModeTarget < 2)
			{
				if (gameModeTarget == 0)
					gGT->gameMode1 |= opcodeMeta->arg1.u;
			}
			else
			{
				if (gameModeTarget == 2)
					gGT->renderFlags |= opcodeMeta->arg1.u;
				else
				{
					if (gameModeTarget == 3)
						gGT->renderFlags &= ~opcodeMeta->arg1.u;
				}
			}
		}
		break;

	case 0x2d:
		cs->Subtitles.textPos.x = opcodeMeta->animIndex;
		cs->Subtitles.textPos.y = opcodeMeta->frameStart;
		cs->Subtitles.lngIndex = opcodeMeta->frameEnd;
		cs->Subtitles.font = opcodeMeta->rotStart;
		cs->Subtitles.colors = opcodeMeta->rotEnd;
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case 0x2e:
		gGT->pushBuffer_UI.fadeFromBlack_desiredResult = 0;
		gGT->pushBuffer_UI.fade_step = 0xfd56;
		CS_ScriptCmd_OpcodeNext(cs);
		goto finishOpcodeStep;

	case 0x2f:
		if (0 < gGT->pushBuffer_UI.fadeFromBlack_currentValue)
			goto updateInstanceAndReturn;
		break;

	case 0x30:
		D233.CutsceneManipulatesAudio = 1;
		howl_VolumeSet(0, (u32) * ((u8 *)opcodeMeta + 2));
		howl_VolumeSet(1, (u32) * ((u8 *)opcodeMeta + 4));
		howl_VolumeSet(2, (u32) * ((u8 *)opcodeMeta + 6));
		break;

	default:
		CS_RestoreDecodedOpcode(cs, metadataBackup);
		return 0;
	}

	CS_ScriptCmd_OpcodeNext(cs);

finishOpcodeStep:
	if ((elapsedTimeRemaining != 0) || (opcodeChanged != 0))
		goto processOpcode;
	goto updateInstanceAndReturn;
}
