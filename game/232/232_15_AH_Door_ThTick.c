#include <common.h>

static char AH_Door_IsOpenByRewards(s16 levelID, s16 doorID)
{
	if ((levelID == N_SANITY_BEACH) && (doorID == 4))
		return (sdata->advProgress.storyFlags & ADV_REWARD_DOOR_BEACH_TO_GLACIER_PARK_MASK) != 0;

	if ((levelID == N_SANITY_BEACH) && (doorID == 5))
		return (sdata->advProgress.storyFlags & ADV_REWARD_DOOR_BEACH_TO_GEMSTONE_VALLEY_MASK) != 0;

	if (levelID == GEM_STONE_VALLEY)
		return (sdata->advProgress.storyFlags & ADV_REWARD_DOOR_GEMSTONE_VALLEY_TO_CUPS_MASK) != 0;

	if (levelID == THE_LOST_RUINS)
		return (sdata->advProgress.storyFlags & ADV_REWARD_DOOR_LOST_RUINS_TO_GLACIER_PARK_MASK) != 0;

	if (levelID == GLACIER_PARK)
		return (sdata->advProgress.storyFlags & ADV_REWARD_DOOR_GLACIER_PARK_TO_CITADEL_CITY_MASK) != 0;

	return false;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 232 0x800afa60-0x800b072c.
void AH_Door_ThTick(struct Thread *t)
{
	char doorIsOpen;
	s16 doorID;
	s16 lev;
	s16 numKeys;
	u16 hintId;
	u32 chkRewards;
	int i;
	int ratio;
	int distX;
	int distZ;
	int dist;
	int iVar17;
	int iVar18;
	s16 desiredPos[3];
	s16 desiredRot[3];
	s16 *scaler;

	struct GameTracker *gGT = sdata->gGT;
	struct WoodDoor *door = t->object;
	struct Instance *doorInst = t->inst;
	struct Instance *keyInst;
	struct Driver *driver = gGT->drivers[0];
	struct Instance *driverInst;
	struct CameraDC *cDC = gGT->cameraDC;

	doorID = door->doorID;

	// Assume door is closed
	doorIsOpen = false;

	lev = gGT->levelID;

	// NOTE(aalhendi): Retail derives open state from adventure rewards here.
	doorIsOpen = AH_Door_IsOpenByRewards(lev, doorID);

	// Cosine(angle)
	ratio = MATH_Cos((int)doorInst->instDef->rot[1]);

	// X distance of player and door
	distX = doorInst->matrix.t[0] + (ratio * 0x300 >> 0xc) - driver->instSelf->matrix.t[0];

	// Sine(angle)
	ratio = MATH_Sin((int)doorInst->instDef->rot[1]);

	// Z distance of player and door
	distZ = doorInst->matrix.t[2] + (ratio * 0x300 >> 0xc) - driver->instSelf->matrix.t[2];

	// distance from player and door
	dist = distX * distX + distZ * distZ;

	// If player is close to a door
	if (dist < 0x90000)
	{
		// if door is open
		if (doorIsOpen)
		{
			// enable access through a door (disable collision)
			sdata->doorAccessFlags |= 2;
		}

		// if door is locked
		else
		{
			// remove access (enable collision)
			sdata->doorAccessFlags &= 0xfffffffd;
		}
	}

	// If this is N Sane Beach
	if (lev == N_SANITY_BEACH)
	{
		// if this is beach -> gemstone,
		// must have 1 key
		numKeys = 1;

		// If this is door from beach -> glacier
		if (doorID == 4)
		{
			// must have 2 keys
			numKeys = 2;
		}
	}
	// If not N Sane Beach
	else
	{
		// get number of keys for whichever door is on the hub
		numKeys = D232.arrKeysNeeded[(lev + -0x19)];
	}

	// if in a state where you're seeing the boss key open an adv door,
	// or some other kind of cutscene where you can't move
	if ((gGT->gameMode2 & 4) != 0)
		return;

	// If door is open
	if (doorIsOpen)
	{
		// camera transition, watch door open
		if ((cDC->flags & CAMERA_FLAG_TRANSITION_AWAY) != 0)
		{
			// quit, come back when camera transition ends
			return;
		}

		// check if hint is unlocked
		chkRewards = CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_NEW_WORLD_GREETING);

		// "Congrats on opening this new area..."
		hintId = ADV_MASK_HINT_ID_NEW_WORLD_GREETING;

	joined_r0x800b06ec:

		// if hint is not unlocked
		if (chkRewards == 0)
		{
			MainFrame_RequestMaskHint(hintId, 0);
		}
		return;
	}

	// == if door is closed ==

	if (
	    // if player is far from the door
	    (0x8ffff < dist) &&
	    // flags
	    ((door->camFlags & WdCam_CutscenePlaying) == 0))
	{
		return;
	}

	// if player has less than that amount
	if (gGT->currAdvProfile.numKeys < numKeys)
	{
		// if one key is required
		if (numKeys == 1)
		{
			// check if hint is unlocked
			chkRewards = CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_MUST_HAVE_ONE_BOSS_KEY);

			// Aku Hint "You must have a boss key"
			hintId = ADV_MASK_HINT_ID_MUST_HAVE_ONE_BOSS_KEY;
		}

		// not one...
		else
		{
			// if not 2...
			if (numKeys != 2)
			{
				// no hint for that
				return;
			}

			// if 2 keys are needed to enter

			// check if hint is unlocked
			chkRewards = CHECK_ADV_BIT(sdata->advProgress.rewards, ADV_REWARD_HINT_MUST_HAVE_TWO_BOSS_KEYS);

			// Aku Hint "You must have two boss keys"
			hintId = ADV_MASK_HINT_ID_MUST_HAVE_TWO_BOSS_KEYS;
		}

		// request hint and quit
		goto joined_r0x800b06ec;
	}

	// == door is closed, ready to unlock ==

	if ((door->camFlags & WdCam_FlyingOut) != 0)
	{
		door->camTimer_unused = 0x3c;

		if (((cDC->flags & CAMERA_FLAG_TRANSITION_AWAY) == 0) && ((door->camFlags & WdCam_FlyingIn) == 0))
		{
			driver->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_Driving_Init;
			door->camFlags |= WdCam_FlyingIn;
		}
		else if (((cDC->flags & CAMERA_FLAG_TRANSITION_HOLD) != 0) && ((door->camFlags & WdCam_FullyOut) == 0))
		{
			door->camFlags |= WdCam_FullyOut;
		}
	}
	else
	{
		if (door->camTimer_unused != 0)
		{
			door->camTimer_unused--;
			return;
		}

		// If the game is paused
		if ((gGT->gameMode1 & 0xf) != 0)
		{
			return;
		}

		// If you are here, game must not be paused

		driver->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_FreezeEndEvent_Init;

		door->camFlags |= WdCam_CutscenePlaying;

		// if timer is less than four full seconds
		if (door->frameCount_doorOpenAnim < 0x78)
		{
			if (driver->speedApprox < 0x80)
			{
				// Actually, specLightDir
				desiredPos[0] = -0xc98;
				desiredPos[1] = 0x99f;
				desiredPos[2] = 0x232;

				// if keys are not spawned, create them
				if (door->keyInst[0] == NULL)
				{
					// if number of keys is more than zero
					if (numKeys > 0)
					{
						// spawn instances for every key you have,
						// this makes them spin in a circle before
						// seeing the doors animate open
						for (i = 0; i < numKeys; i++)
						{
							// name = "key"
							keyInst = INSTANCE_Birth3D(gGT->modelPtr[STATIC_KEY], R232.s_key, t);

							// Set Key Color
							keyInst->colorRGBA = 0xdca6000;

							// specular lighting
							keyInst->flags |= 0x20000;
							door->frameCount_unused++;

							driverInst = driver->instSelf;
							keyInst->matrix = driverInst->matrix;

							// set scale to zero
							keyInst->scale[0] = 0;
							keyInst->scale[1] = 0;
							keyInst->scale[2] = 0;

							door->keyInst[i] = keyInst;
						}
					}

					// Make a backup of HUD variable
					door->hudFlags = (u8)gGT->hudFlags;

					// Disable HUD
					gGT->hudFlags = 0;

					// For watching hub doors animate open
				}

				// if more than zero
				if (0 < numKeys)
				{
					// loop through all keys
					for (i = 0; i < numKeys; i++)
					{
						keyInst = door->keyInst[i];
						if (keyInst != NULL)
						{
							// if scale < 0xa00
							if (keyInst->scale[0] < 0xa00)
							{
								// increase scale on X, Y, Z
								keyInst->scale[0] += 0x40;
								keyInst->scale[1] += 0x40;
								keyInst->scale[2] += 0x40;
							}

							// if key posY is less than (player posY + 0xa0)
							if (keyInst->matrix.t[1] < (driver->instSelf->matrix.t[1] + 0xa0))
							{
								// increase key posY
								keyInst->matrix.t[1] += 4;
							}

							if (1 < numKeys)
							{
								iVar18 = i * (0x1000 / numKeys);
								iVar17 = (int)keyInst->scale[0];

								if (iVar17 < 0)
								{
									iVar17 += 0x1f;
								}

								ratio = MATH_Sin(door->keyOrbit + iVar18);

								keyInst->matrix.t[0] = driver->instSelf->matrix.t[0] + ((iVar17 >> 5) * ratio >> 0xc);

								ratio = MATH_Cos(door->keyOrbit + iVar18);

								keyInst->matrix.t[2] = driver->instSelf->matrix.t[2] + ((iVar17 >> 5) * ratio >> 0xc);
							}

							s16 *kr = &door->keyRot[0];

							// desiredPos is actually specLightDir in this case, variable re-use
							Vector_SpecLightSpin3D(keyInst, kr, &desiredPos[0]);

							// convert 3 rotation shorts into rotation matrix
							ConvertRotToMatrix(&keyInst->matrix, kr);
						}
						door->keyInst[i] = keyInst;
					}
				}

				door->keyRot[0] = 0;
				door->keyRot[1] += 0x40;
				door->keyRot[2] = 0;

				door->keyOrbit += 0x10;

				door->frameCount_doorOpenAnim++;

				// Sound effects when keys float in air

				switch (door->frameCount_doorOpenAnim)
				{
				case 0x0A:
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0208-0x800b0218 for first floating-key SFX.
					OtherFX_Play_LowLevel(0x67, 1, 0xff7680);
					break;
				case 0x0F:
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b022c-0x800b023c for second floating-key SFX.
					OtherFX_Play_LowLevel(0x67, 1, 0xeb8080);
					break;
				case 0x14:
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0250-0x800b0260 for third floating-key SFX.
					OtherFX_Play_LowLevel(0x67, 1, 0xd78a80);
					break;
				case 0x19:
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0274-0x800b0284 for fourth floating-key SFX.
					OtherFX_Play_LowLevel(0x67, 1, 0xc39480);
					break;
				case 0x50:
					// unlock door sound
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0298-0x800b02ac for door unlock SFX.
					OtherFX_Play(0x93, 1);
					break;
				case 0x78:
					// on last frame, doors "creek" open
					// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b02a0-0x800b02ac for door creak SFX.
					OtherFX_Play(0x94, 1);
					break;

				default:
					break;
				}
			}
			return;
		}

		// == After 4 seconds ==

		ratio = MATH_Cos((int)doorInst->instDef->rot[1]);

		i = MATH_Cos((int)doorInst->instDef->rot[1] + 0x400);

		// desired posX for transition
		desiredPos[0] = doorInst->matrix.t[0] + (s16)(ratio * 0x312 >> 0xc) + (s16)(i * 0x600 >> 0xc);
		// desired posY for transition
		desiredPos[1] = doorInst->matrix.t[1] + 0x17a;

		ratio = MATH_Sin((int)doorInst->instDef->rot[1]);

		i = MATH_Sin((int)doorInst->instDef->rot[1] + 0x400);

		// desired posZ for transition
		desiredPos[2] = doorInst->matrix.t[2] + (s16)(ratio * 0x312 >> 0xc) + (s16)(i * 0x600 >> 0xc);

		// desired rotation for transition
		desiredRot[0] = doorInst->instDef->rot[0] + 0x800;
		desiredRot[1] = doorInst->instDef->rot[1];
		desiredRot[2] = doorInst->instDef->rot[2];

		// set desired position and rotation for CamerDC transition
		CAM_SetDesiredPosRot(&gGT->cameraDC[0], &desiredPos[0], &desiredRot[0]);

		GAMEPAD_JogCon2(driver, 0, 0);

		// start camera out transition (in "else" below)
		door->camFlags |= WdCam_FlyingOut;

		return;
	}

	// == door is opening ==

	if (door->doorRot[1] < 0x400)
	{
		door->doorRot[1] += 0x10;

		// right-hand door rot[x,y,z]
		desiredRot[0] = door->doorRot[0];
		desiredRot[1] = doorInst->instDef->rot[1] - door->doorRot[1];
		desiredRot[2] = door->doorRot[2];

		// converted to TEST in rebuildPS1
		ConvertRotToMatrix(&door->otherDoor->matrix, &desiredRot[0]);

		// left-hand door rot[x,y,z]
		desiredRot[1] = doorInst->instDef->rot[1] + door->doorRot[1];

		// converted to TEST in rebuildPS1
		ConvertRotToMatrix(&doorInst->matrix, &desiredRot[0]);

		// if less than 11 frames have passed,
		// decrease key scale, then quit function
		if (door->keyShrinkFrame < 0xb)
		{
			scaler = (s16 *)R232.keyFrame;

			// loop through 4 keys
			for (i = 0; i < 4; i++)
			{
				keyInst = door->keyInst[i];
				// if instance exists
				if (keyInst != NULL)
				{
					// decrease scale of key
					keyInst->scale[0] = scaler[door->keyShrinkFrame];
					keyInst->scale[1] = scaler[door->keyShrinkFrame];
					keyInst->scale[2] = scaler[door->keyShrinkFrame];
				}
			}

			door->keyShrinkFrame++;

			return;
		}

		// loop through 4 keys
		for (i = 0; i < 4; i++)
		{
			INSTANCE_Death(door->keyInst[i]);
			door->keyInst[i] = NULL;
		}

		return;
	}

	// == Door is fully open ==

	if (
	    // if this is N Sane Beach
	    ((lev == N_SANITY_BEACH) &&

	     // if this is door #4 (beach -> glacier)
	     (doorID == 4)) ||

	    // if this is lost ruins (ruins -> glacier)
	    (lev == THE_LOST_RUINS))
	{
		// open all doors to glacier
		sdata->advProgress.storyFlags |= ADV_REWARD_DOORS_TO_GLACIER_PARK_MASK;
	}

	else if (
	    // if this is N Sane Beach
	    (lev == N_SANITY_BEACH) &&

	    // Door #5 (beach -> ruins)
	    (doorID == 5))
	{
		// record that door is open
		sdata->advProgress.storyFlags |= ADV_REWARD_DOOR_BEACH_TO_GEMSTONE_VALLEY_MASK;
	}

	// Gemstone valley (cup door)
	else if (lev == GEM_STONE_VALLEY)
	{
		// record that door is open
		sdata->advProgress.storyFlags |= ADV_REWARD_DOOR_GEMSTONE_VALLEY_TO_CUPS_MASK;
	}

	// Glacier Park (glacier -> citadel)
	else
	{
		// record that door is open
		sdata->advProgress.storyFlags |= ADV_REWARD_DOOR_GLACIER_PARK_TO_CITADEL_CITY_MASK;
	}

	cDC->flags |= CAMERA_FLAG_TRANSITION_BACK;

	driver->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_Driving_Init;

	// cutscene over
	door->camFlags = (door->camFlags & ~WdCam_CutscenePlaying) | WdCam_FlyingIn;

	// bring HUD back
	gGT->hudFlags = (u8)door->hudFlags;
}
