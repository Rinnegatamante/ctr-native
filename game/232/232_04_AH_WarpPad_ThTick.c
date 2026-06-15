#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abf48-0x800ad2c8.
void AH_WarpPad_ThTick(struct Thread *t)
{
	int i;
	int j;
	int boolOpen;
	struct GameTracker *gGT;
	struct WarpPad *warppadObj;
	struct Instance *warppadInst;
	struct Instance **visInstSrc;
	struct Instance **instArr;

	struct Driver *driver;
	struct Instance *driverInst;

	int modelID;
	int levelID;
	int x, y, z, dist;
	char *warppadLNG;

	int angleCamToWarppad;
	int angleSin, angleCos;
	MATRIX *warppadMatrix;

	int wispMaxHeight;
	int wispRiseRate;
	int rng1;
	int rng2;

	int rewardScale;
	int rewardScale2;

	int champID;
	int champSlot;

	char randKartSpawn[8];

	// NOTE(aalhendi): WarpPad level IDs come from "warppad#NN" instance names
	// and use retail adventure numbering, not the native LevelID enum.
	enum
	{
		AH_WP_SLIDE_COLISEUM = 16,
		AH_WP_TURBO_TRACK = 17,
		AH_WP_NITRO_COURT = 18,
		AH_WP_GEM_STONE_VALLEY = 25,
		AH_WP_ADV_CUP = 100,
	};

	boolOpen = 0;
	gGT = sdata->gGT;
	warppadObj = t->object;
	warppadInst = t->inst;
	visInstSrc = gGT->cameraDC[0].visInstSrc;

#if defined(CTR_NATIVE)
	// NOTE(aalhendi): Retail can read PS1 low RAM when the hub-swap frame
	// leaves this list null; native treats that as an empty visible-instance list.
	if (visInstSrc != NULL)
#endif
	{
		while (visInstSrc[0] != 0)
		{
			if (visInstSrc[0] == warppadInst)
			{
				boolOpen = 1;
				break;
			}

			visInstSrc++;
		}
	}

	// array of instances in warppad object
	instArr = &warppadObj->inst[0];
	warppadMatrix = &warppadInst->matrix;

	// make instances visible
	if (boolOpen == 1)
	{
		for (i = 0; i < WPIS_NUM_INSTANCES; i++)
			if (instArr[i] != 0)
				instArr[i]->flags &= ~(0x80);
	}

	// make instances invisible
	else
	{
		for (i = 0; i < WPIS_NUM_INSTANCES; i++)
			if (instArr[i] != 0)
				instArr[i]->flags |= 0x80;
	}

	// This is the red triangle in DCxDemo's
	// level viewer, make it invisible.
	// Instance only exists for debugging
	warppadInst->flags |= 0x80;

	driver = gGT->drivers[0];
	driverInst = driver->instSelf;

	x = warppadMatrix->t[0] - driverInst->matrix.t[0];
	y = warppadMatrix->t[1] - driverInst->matrix.t[1];
	z = warppadMatrix->t[2] - driverInst->matrix.t[2];
	dist = x * x + y * y + z * z;

	levelID = warppadObj->levelID;

	// if near a portal
	if (
	    // Trophy tracks (-16)
	    ((levelID < AH_WP_SLIDE_COLISEUM) && (dist < 0x144000)) ||

	    // Slide Col + Turbo Track (-16)
	    ((((u16)(levelID - AH_WP_SLIDE_COLISEUM)) < 2) && (dist < 0x90000)) ||

	    // Battle tracks (-18)
	    ((((u16)(levelID - AH_WP_NITRO_COURT)) < 7) && (dist < 0x144000)) ||

	    // Gem cups
	    ((levelID >= AH_WP_ADV_CUP) && (dist < 0x90000)))
	{
		// if you are near a new warppad, or if you already were
		// determined as near the same warppad in the last frame,
		// then use this warppad as the "closest". Otherwise the
		// game could run this for two warppads right next to each other
		if ((D232.levelID == -1) || (D232.levelID == levelID))
		{
			// saved as nearest warppad
			D232.levelID = levelID;


			// if not giving Aku Hint
			if (sdata->AkuAkuHintState == 0)
			{
				// default
				if (levelID < AH_WP_ADV_CUP)
					warppadLNG = sdata->lngStrings[data.metaDataLEV[levelID].name_LNG];

				// gem cups
				else
					warppadLNG = sdata->lngStrings[data.AdvCups[levelID - AH_WP_ADV_CUP].lngIndex_CupName];

				// midpoing X,
				// 30 pixels above botttom Y
				DecalFont_DrawLine(warppadLNG, gGT->pushBuffer[0].rect.x + gGT->pushBuffer[0].rect.w / 2,
				                   gGT->pushBuffer[0].rect.y + gGT->pushBuffer[0].rect.h - 30, FONT_BIG, (JUSTIFY_CENTER | ORANGE));
			}

			// if track is unlocked, ignore all other ELSE-IFs
			if (instArr[WPIS_CLOSED_1S] == 0)
			{
			}

			else if (

			    // gem cup
			    (levelID >= AH_WP_ADV_CUP) &&

			    // Dont have hint "you must have 4 tokens for a gem"
			    (CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_GEM_CUPS_CHALLENGE) == 0)

			)
			{
				// give hint "you must have 4 tokens for a gem"
				MainFrame_RequestMaskHint(ADV_MASK_HINT_ID_GEM_CUPS_CHALLENGE, 0);
			}

			else if (

			    // Trophy track
			    (levelID < AH_WP_SLIDE_COLISEUM) &&

			    // Dont have hint "you must have more trophies"
			    (CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_NEED_MORE_TROPHIES) == 0) &&

			    // required item is not KEY
			    (instArr[WPIS_CLOSED_ITEM]->model->id != STATIC_KEY))
			{
				// give hint for "need more trophies"
				MainFrame_RequestMaskHint(ADV_MASK_HINT_ID_NEED_MORE_TROPHIES, 0);
			}

			else if (

			    // Slide Col
			    (levelID == AH_WP_SLIDE_COLISEUM) &&

			    // Dont have hint "you must have 10 relics"
			    (CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_MUST_GET_10_RELICS) == 0))
			{
				// give hint for "need more trophies"
				MainFrame_RequestMaskHint(ADV_MASK_HINT_ID_MUST_GET_10_RELICS, 0);
			}
		}
	}

	// not near portal
	else
	{
		D232.levelID = -1;
	}

	// if warppad is locked
	if (instArr[WPIS_CLOSED_1S] != 0)
	{
		angleCamToWarppad = ratan2(warppadMatrix->t[0] - gGT->pushBuffer[0].pos[0], warppadMatrix->t[2] - gGT->pushBuffer[0].pos[2]);

		angleCamToWarppad = -angleCamToWarppad;

		angleSin = MATH_Sin(angleCamToWarppad);
		angleCos = MATH_Cos(angleCamToWarppad);

		// no 10s digit
		if (instArr[WPIS_CLOSED_10S] == 0)
		{
			instArr[WPIS_CLOSED_1S]->matrix.t[0] = warppadMatrix->t[0] + (angleCos * -0x80 >> 0xC);
			instArr[WPIS_CLOSED_1S]->matrix.t[2] = warppadMatrix->t[2] + (angleSin * -0x80 >> 0xC);

			instArr[WPIS_CLOSED_ITEM]->matrix.t[0] = warppadMatrix->t[0] + ((angleCos << 7) >> 0xC);
			instArr[WPIS_CLOSED_ITEM]->matrix.t[2] = warppadMatrix->t[2] + ((angleSin << 7) >> 0xC);
		}

		// 10s digit
		else
		{
			instArr[WPIS_CLOSED_ITEM]->matrix.t[0] = warppadMatrix->t[0] + (angleCos * 0xC0 >> 0xC);
			instArr[WPIS_CLOSED_ITEM]->matrix.t[2] = warppadMatrix->t[2] + (angleSin * 0xC0 >> 0xC);

			instArr[WPIS_CLOSED_X]->matrix.t[0] = warppadMatrix->t[0] + ((angleCos << 6) >> 0xC);
			instArr[WPIS_CLOSED_X]->matrix.t[2] = warppadMatrix->t[2] + ((angleSin << 6) >> 0xC);

			instArr[WPIS_CLOSED_10S]->matrix.t[0] = warppadMatrix->t[0] + (angleCos * -0x40 >> 0xC);
			instArr[WPIS_CLOSED_10S]->matrix.t[2] = warppadMatrix->t[2] + (angleSin * -0x40 >> 0xC);

			instArr[WPIS_CLOSED_1S]->matrix.t[0] = warppadMatrix->t[0] + (angleCos * -0xa0 >> 0xC);
			instArr[WPIS_CLOSED_1S]->matrix.t[2] = warppadMatrix->t[2] + (angleSin * -0xa0 >> 0xC);
		}

		warppadObj->spinRot_Prize[0] = 0;
		warppadObj->spinRot_Prize[2] = 0;

		warppadObj->spinRot_Prize[1] += 0x40;

		// reuse variable,
		// end of function anyway
		warppadInst = instArr[WPIS_CLOSED_ITEM];
#define InstArr0 warppadInst

		// converted to TEST in rebuildPS1
		ConvertRotToMatrix(&InstArr0->matrix, &warppadObj->spinRot_Prize[0]);

		modelID = InstArr0->model->id;

		// Trophy has no specular light
		if (modelID == STATIC_TROPHY)
			return;

		// NOTE(aalhendi): Retail passes the per-WarpPad spec-light arrays at
		// offsets 0x50/0x58/0x60.

		// Relic
		if (modelID == STATIC_RELIC)
		{
			Vector_SpecLightSpin3D(InstArr0, &warppadObj->spinRot_Prize[0], &warppadObj->specLightRelic[0]);
			return;
		}

		// Token
		if (modelID == STATIC_TOKEN)
		{
			Vector_SpecLightSpin3D(InstArr0, &warppadObj->spinRot_Prize[0], &warppadObj->specLightToken[0]);
			return;
		}

		// If Gem, change colors every 2 seconds
		if (modelID == STATIC_GEM)
		{
			i = (gGT->timer / 0x3C) % 5;

			InstArr0->colorRGBA = ((u32)data.AdvCups[i].color[0] << 0x14) | ((u32)data.AdvCups[i].color[1] << 0xc) | ((u32)data.AdvCups[i].color[2] << 0x4);
		}

		// for Key or Gem
		Vector_SpecLightSpin3D(InstArr0, &warppadObj->spinRot_Prize[0], &warppadObj->specLightGem[0]);
		return;
	}

	// === Assume Unlocked ===

	if ((dist > 0x8fff) && (warppadObj->boolEnteredWarppad == 0))
		goto WarpPad_AnimateOpen;

	// Retail repeats this setup every close/warping frame before the
	// transition/load gate.
	LOAD_Robots1P(data.characterIDs[0]);

	// variable reuse, get track speed champion
	champID = data.metaDataLEV[levelID].characterID_Champion;

	// default
	champSlot = 0;

	// If Speed Champion is on the track (Crash-Pura)
	// and is not the same characterID as this driver
	if ((champID < 8) && (champID != data.characterIDs[driver->driverID]))
	{
		// set everyone to spawn in order
		for (i = 1; i < 8; i++)
		{
			if (champID == data.characterIDs[i])
			{
				sdata->kartSpawnOrderArray[i] = 0;
				champSlot = i;
			}

			else if (i == 7)
			{
				sdata->kartSpawnOrderArray[7] = champSlot;
			}

			else
			{
				sdata->kartSpawnOrderArray[i] = i;
			}
		}
	}

	// Speed Champion is invalid
	else
	{
		for (i = 1; i < 8; i++)
			randKartSpawn[i] = i;

		for (i = 0; i < 7; i++)
		{
			rng1 = RngDeadCoed(&sdata->const_0x30215400);

			rng2 = 7 - i;

			rng2 = (rng1 & 0xfff) % rng2 + 1;
			rng2 = (s16)rng2;

			sdata->kartSpawnOrderArray[randKartSpawn[rng2]] = (char)i;

			while (rng2 < 7)
			{
				randKartSpawn[rng2] = randKartSpawn[rng2 + 1];
				rng2++;
			}
		}
	}

	// spawn P1 in the back
	sdata->kartSpawnOrderArray[0] = 7;

	// if flag is on-screen, loading has already been finalized
	if (RaceFlag_IsTransitioning() != 0)
		goto WarpPad_AnimateOpen;

	levelID = warppadObj->levelID;

	// gem cups
	if (levelID >= AH_WP_ADV_CUP)
	{
		warppadObj->boolEnteredWarppad = 1;
		warppadObj->framesWarping++;
		gGT->drivers[0]->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_Warp_Init;
		if (warppadObj->framesWarping < 61)
			goto WarpPad_AnimateOpen;

		sdata->Loading.OnBegin.AddBitsConfig0 |= ADVENTURE_CUP;

		gGT->cup.cupID = levelID - AH_WP_ADV_CUP;
		gGT->cup.trackIndex = 0;
		for (i = 0; i < 8; i++)
			gGT->cup.points[i] = 0;

		levelID = data.advCupTrackIDs[4 * gGT->cup.cupID];
		goto WarpPad_RequestLoad;
	}

	// Slide Col or Turbo Track
	if (((u16)(levelID - AH_WP_SLIDE_COLISEUM)) < 2)
	{
		warppadObj->boolEnteredWarppad = 1;
		warppadObj->framesWarping++;
		gGT->drivers[0]->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_Warp_Init;
		if (warppadObj->framesWarping < 61)
			goto WarpPad_AnimateOpen;

		sdata->Loading.OnBegin.AddBitsConfig0 |= RELIC_RACE;
		goto WarpPad_RequestLoad;
	}

	// Battle Tracks
	if ((((u16)(levelID - AH_WP_NITRO_COURT)) < 2) || (levelID == 21) || (levelID == 23))
	{
		warppadObj->boolEnteredWarppad = 1;
		warppadObj->framesWarping++;
		gGT->drivers[0]->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_Warp_Init;
		if (warppadObj->framesWarping < 61)
			goto WarpPad_AnimateOpen;

		sdata->Loading.OnBegin.AddBitsConfig0 |= CRYSTAL_CHALLENGE;

		// Dont have hint "collect every crystal"
		if (CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_COLLECT_EVERY_CRYSTAL) == 0)
			MainFrame_RequestMaskHint(ADV_MASK_HINT_ID_COLLECT_EVERY_CRYSTAL, 1);

		// if can't spawn aku cause he's already here,
		// quit function, wait till he's done to start race
		i = AH_MaskHint_boolCanSpawn();
		if ((i & 0xffff) == 0)
			goto WarpPad_AnimateOpen;

		gGT->originalEventTime = D232.timeCrystalChallenge[levelID - AH_WP_NITRO_COURT];
		goto WarpPad_RequestLoad;
	}

	if (levelID < AH_WP_SLIDE_COLISEUM)
	{
		if (CHECK_ADV_BIT(sdata->advProgress.rewards, levelID + ADV_REWARD_FIRST_TROPHY) != 0)
		{
			if (gGT->currAdvProfile.numTrophies >= data.metaDataLEV[levelID].numTrophiesToOpen)
			{
				if (warppadObj->framesWarping < 61)
					goto WarpPad_TrophyAnimateOnly;

				// if never opened
				if (sdata->boolOpenTokenRelicMenu == 0)
				{
					if ((gGT->gameMode1 & ADVENTURE_ARENA) != 0)
					{
						D232.menuTokenRelic.rowSelected = (CHECK_ADV_BIT(sdata->advProgress.rewards, levelID + ADV_REWARD_FIRST_CTR_TOKEN) != 0);

						RECTMENU_Show(&D232.menuTokenRelic);

						// now opened
						sdata->boolOpenTokenRelicMenu = 1;
					}
				}

				// if opened, but not closed yet
				if ((RECTMENU_BoolHidden(&D232.menuTokenRelic) & 0xffff) == 0)
					goto WarpPad_TrophyAnimateOnly;

				// Relic Hint
				i = ADV_MASK_HINT_ID_RELIC_CHALLENGE;

				// CTR Token Hint
				if ((gGT->gameMode2 & 8) != 0)
					i = ADV_MASK_HINT_ID_CTR_TOKEN_CHALLENGE;

				// if hint is locked
				if (CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_FIRST_HINT + i) == 0)
					MainFrame_RequestMaskHint(i, 1);

				// if can't spawn aku cause he's already here,
				// quit function, wait till he's done to start race
				i = AH_MaskHint_boolCanSpawn();
				if ((i & 0xffff) == 0)
					goto WarpPad_TrophyAnimateOnly;

				// reset for future gameplay
				sdata->boolOpenTokenRelicMenu = 0;
				warppadObj->boolEnteredWarppad = 0;

				// Rem Adventure Arena
				sdata->Loading.OnBegin.RemBitsConfig0 |= ADVENTURE_ARENA;

				MainRaceTrack_RequestLoad(levelID);
				goto WarpPad_TrophyAnimateOnly;
			}
		}
	}

	if (CHECK_ADV_BIT(sdata->advProgress.rewards, levelID + ADV_REWARD_FIRST_TROPHY) != 0)
	{
		i = data.metaDataLEV[levelID].hubID + ADV_REWARD_BOSS_KEY_HUB_ID_BASE;

		if (CHECK_ADV_BIT(sdata->advProgress.rewards, i) == 0)
			goto WarpPad_AnimateOpen;
	}

	warppadObj->boolEnteredWarppad = 1;
	warppadObj->framesWarping++;
	gGT->drivers[0]->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_Warp_Init;
	if (warppadObj->framesWarping < 61)
		goto WarpPad_AnimateOpen;

WarpPad_RequestLoad:

	// Rem Adventure Arena
	sdata->Loading.OnBegin.RemBitsConfig0 |= ADVENTURE_ARENA;

	MainRaceTrack_RequestLoad(levelID);
	goto WarpPad_AnimateOpen;

WarpPad_TrophyAnimateOnly:

	if (warppadObj->framesWarping < 0x400)
		warppadObj->framesWarping++;

	warppadObj->boolEnteredWarppad = 1;

	gGT->drivers[0]->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_Warp_Init;

WarpPad_AnimateOpen:

	if ((instArr[WPIS_OPEN_BEAM] != 0) && ((gGT->timer & 1) != 0))
	{
		warppadObj->spinRot_Beam[0] = 0;
		warppadObj->spinRot_Beam[2] = 0;

		// what on earth was this RNG?
		// how'd they come up with something so random, that looks so good?
		i = MixRNG_Scramble();
		warppadObj->spinRot_Beam[1] += ((s16)(i >> 3) + (s16)((i >> 3) / 6) * -6 + 1) * 0x200;

		// converted to TEST in rebuildPS1
		ConvertRotToMatrix(&instArr[WPIS_OPEN_BEAM]->matrix, &warppadObj->spinRot_Beam[0]);
	}

	wispRiseRate = 0x20;

	wispMaxHeight = 0x600;

	// if close to this warppad
	if (D232.levelID != -1)
		wispMaxHeight = 0x400;

	for (i = 0; i < 2; i++)
	{
		if (instArr[WPIS_OPEN_RING1 + i] != 0)
		{
			warppadObj->spinRot_Wisp[i][0] = 0;
			warppadObj->spinRot_Wisp[i][2] = 0;

			warppadObj->spinRot_Wisp[i][1] += 0x100;

			// converted to TEST in rebuildPS1
			ConvertRotToMatrix(&instArr[WPIS_OPEN_RING1 + i]->matrix, &warppadObj->spinRot_Wisp[i][0]);

			// if height hasn't reached max height
			if (instArr[WPIS_OPEN_RING1 + i]->matrix.t[1] < (warppadInst->matrix.t[1] + wispMaxHeight))
			{
				instArr[WPIS_OPEN_RING1 + i]->matrix.t[1] += wispRiseRate;

				// if height hasn't reached 4x RiseRate,
				// first 4 frames of rising
				if (instArr[WPIS_OPEN_RING1 + i]->matrix.t[1] < (warppadInst->matrix.t[1] + wispRiseRate * 4))
				{
					// reduce transparency
					instArr[WPIS_OPEN_RING1 + i]->alphaScale -= 0x380;
				}

				// after first 4 frames
				else
				{
					// add transparency as the wisp spirals upward (~0x60  per frame)
					instArr[WPIS_OPEN_RING1 + i]->alphaScale += 0xc00 / (wispMaxHeight / wispRiseRate);
				}
			}

			// eached max height
			else
			{
				// reset height
				instArr[WPIS_OPEN_RING1 + i]->matrix.t[1] = warppadInst->matrix.t[1];

				// full transparency
				instArr[WPIS_OPEN_RING1 + i]->alphaScale = 0x1000;

				rng1 = MixRNG_Scramble() >> 3;

				rng2 = rng1;
				if (rng1 < 0)
					rng2 = rng1 + 0xfff;

				warppadObj->spinRot_Wisp[i][1] = (s16)rng1 + (s16)(rng2 >> 0xc) * -0x1000;
			}
		}

		wispRiseRate += 0x10;
	}

	warppadObj->spinRot_Prize[1] += 0x80;

	rewardScale = 0x100;

	if (dist > 0x900000 * 2)
	{
		rewardScale = 0;
	}

	else if (dist > 0x900000)
	{
		// range [90, 90*2] to [0%, 100%]
		rewardScale = ((((0x900000 * 2) - dist) * 0x100) / 0x900000);
	}

	for (i = 0; i < 3; i++)
	{
		warppadObj->spinRot_Prize[2] = 0x155;

		if (instArr[WPIS_OPEN_PRIZE1 + i] != 0)
		{
			AH_WarpPad_SpinRewards(instArr[WPIS_OPEN_PRIZE1 + i], warppadObj, i, warppadInst->matrix.t[0], warppadInst->matrix.t[1], warppadInst->matrix.t[2]);

			modelID = instArr[WPIS_OPEN_PRIZE1 + i]->model->id;

			if (rewardScale == 0)
			{
				// invisible
				instArr[WPIS_OPEN_PRIZE1 + i]->flags |= 0x80;
			}

			else
			{
				// visible
				instArr[WPIS_OPEN_PRIZE1 + i]->flags &= ~(0x80);

				// token
				rewardScale2 = 0x2000;

				// not token
				if (modelID != STATIC_TOKEN)
				{
					// trophy
					rewardScale2 = 0x2800;

					// relic
					if (modelID == STATIC_RELIC)
					{
						rewardScale2 = 0x1800;
					}
				}

				rewardScale2 = (u32)(rewardScale2 * rewardScale) >> 8;
				instArr[WPIS_OPEN_PRIZE1 + i]->scale[0] = (s16)rewardScale2;
				instArr[WPIS_OPEN_PRIZE1 + i]->scale[1] = (s16)rewardScale2;
				instArr[WPIS_OPEN_PRIZE1 + i]->scale[2] = (s16)rewardScale2;
			}
		}

		warppadObj->thirds[i] += 0x20;
		warppadObj->spinRot_Rewards[1] += 0x4;
	}

	if (instArr[WPIS_CLOSED_1S] != 0)
	{
		INSTANCE_Death(instArr[WPIS_CLOSED_1S]);
		INSTANCE_Death(instArr[WPIS_CLOSED_10S]);
		INSTANCE_Death(instArr[WPIS_CLOSED_X]);
		INSTANCE_Death(instArr[WPIS_CLOSED_ITEM]);
	}
}
