#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80026ed8-0x80027838.
void GhostReplay_ThTick(struct Thread *t)
{
	struct GhostTape *tape;
	struct GameTracker *gGT;
	struct GhostHeader *gh;
	s16 opcodePos;
	int packetIdx;
	u32 scaledPacketIdx;
	u16 lerp4096;
	u32 delta;
	u8 *packetPtr;
	struct Instance *inst;
	struct Driver *d;
	struct GhostPacket *packet;
	s16 local_rot[3]; // ushort?
	int timeInRace;
	int scaledNum;
	int color;

	d = t->object;
	tape = d->ghostTape;
	inst = d->instSelf;

	inst->scale[0] = 0xccc;
	inst->scale[1] = 0xccc;
	inst->scale[2] = 0xccc;

	// 6-second timer != 0, and ghost made by human
	if ((sdata->ghostOverflowTextTimer != 0) && (d->ghostID == 0))
	{
		color = 0xFFFF8004;
		if (sdata->ghostOverflowTextTimer & 1)
		{
			color = 0xFFFF8003;
		}

		// GHOST DATA OVERFLOW
		// CAN NOT SAVE GHOST DATA
		DecalFont_DrawLine(sdata->lngStrings[361], 0x100, 0x28, 2, color);
		DecalFont_DrawLine(sdata->lngStrings[362], 0x100, 0x32, 2, color);

		sdata->ghostOverflowTextTimer--;
	}

	gGT = sdata->gGT;

	if ((sdata->boolGhostsDrawing == 0) || ((gGT->gameMode1 & DEBUG_MENU) != 0) || (tape->ptrEnd == tape->ptrStart) || (d->ghostBoolInit == 0))
	{
		inst->flags |= HIDE_MODEL;
		return;
	}

	if (d->reserves > 0)
	{
		d->reserves -= gGT->elapsedTimeMS;
		if (d->reserves < 0)
			d->reserves = 0;
	}

	if ((gGT->trafficLightsTimer < 1) && (d->ghostBoolStarted == 0))
	{
		d->ghostBoolStarted = 1;
		tape->packetID = -1;
	}

	inst->alphaScale = 0xa00;

	// remove flags + add transparency
	inst->flags = (inst->flags & 0xfff8ff7f) | GHOST_DRAW_TRANSPARENT;

	timeInRace = tape->timeElapsedInRace >= 0 ? tape->timeElapsedInRace : 0;

	packet = &tape->packets[0];

	// flush and rewrite cached GhostPackets array
	if (tape->timeInPacket32 <= timeInRace)
	{
		opcodePos = 0;
		packetPtr = tape->ptrCurr;
		s16 tmpPos[3] = {0};

		char *packetEndChain = tape->ptrCurr;

		tape->packetID = -1;
		tape->timeInPacket01 = tape->timeInPacket32_backup;

		// move two POSITION(0x80) opcodes in advance,
		// combine with velocity to make GhostPackets cache
		while (opcodePos < 2)
		{
			// reached end of tape
			if (tape->ptrEnd <= (void *)packetPtr)
			{
				gh = tape->gh;

				d->ySpeed = gh->ySpeed;
				d->actionsFlagSet &= 0xffefffff; // driver is not AI anymore
				d->speedApprox = gh->speedApprox;

				BOTS_Driver_Convert(d);
				BOTS_ThTick_Drive(t);

				// 26th bit -> (on) := racer finished race
				d->actionsFlagSet |= 0x2000000;

				// allow this thread to ignore all collisions
				t->flags |= 0x1000;
				return;
			}

			// if opcode is seen
			u32 opcode = (u32)packetPtr[0];
			if ((opcode + 0x80 & 0xff) < 5)
			{
				switch (opcode)
				{
				case 0x80: // position data
					for (int i = 0; i < 3; ++i)
					{
						// Little Endian to Big Endian
						u16 rawValue = (u16)((packetPtr[1 + i * 2] << 8) | packetPtr[2 + i * 2]);

						tmpPos[i] = (s16)(((int)((u32)rawValue << 0x10)) >> 0xd);
						packet->pos[i] = tmpPos[i];
					}

					packet->rot[0] = 0;

					// yes, this is correct
					packet->rot[1] = (u16)packetPtr[9] << 4;
					packet->rot[2] = (u16)packetPtr[10] << 4;

					// if 2nd position opcode
					if (opcodePos == 1)
					{
						// Get time (big endian) from position message
						int bigEndianTime = (packetPtr[7] << 8) | packetPtr[8];
						tape->ptrCurr = packetPtr;

						// casting required, or only half register is
						// written to bigEndianTime, which breaks timeInPacket
						tape->timeInPacket32_backup += bigEndianTime;
						tape->timeInPacket32 += bigEndianTime;
					}

					// count position opcodes
					opcodePos++;

					packet->bufferPacket = packetEndChain;
					packetPtr += 11;

					// the end of the chain represents the last byte
					// that can be checked in the 32 packets, can be
					// used to search for animation opcodes between
					// position packets
					packetEndChain = packetPtr;

					packet++;

					break;

				case 0x81: // animation flags
					packetPtr += 3;
					break;

				case 0x82: // boost flags
					packetPtr += 6;
					break;

				case 0x83: // instance flags
					packetPtr += 2;
					break;

				case 0x84: // driver does nothing
					for (int i = 0; i < 3; ++i)
					{
						packet->pos[i] = tmpPos[i];
					}

					for (int i = 0; i < 3; ++i)
					{
						packet[0].rot[i] = packet[-1].rot[i];
					}

					packet->bufferPacket = packetEndChain;
					packetPtr += 1;
					packetEndChain = packetPtr;

					packet++;
					break;
				}
			}

			// if no opcode, assume 5 bytes of velocity
			else
			{
				for (int i = 0; i < 3; ++i)
				{
					tmpPos[i] += (s16)((char)packetPtr[i]) * 8;
					packet->pos[i] = tmpPos[i];
				}

				packet->rot[0] = 0;

				// yes, this is right
				packet->rot[1] = packetPtr[3] << 4;
				packet->rot[2] = packetPtr[4] << 4;

				packet->bufferPacket = packetEndChain;
				packetPtr += 5;
				packetEndChain = packetPtr;

				packet++;
			}
		}

		tape->numPacketsInArray = ((u32)packet - (u32)&tape->packets[0]) >> 4;

		tape->numPacketsInArray -= 1;

		if (tape->numPacketsInArray < 0)
		{
			tape->numPacketsInArray = 1;
		}

		tape->timeBetweenPackets = tape->timeInPacket32 - tape->timeInPacket01;
		if (tape->timeBetweenPackets == 0)
		{
			tape->timeBetweenPackets = 1;
		}
	}

	scaledNum = (timeInRace - tape->timeInPacket01) * tape->numPacketsInArray * 0x1000;

#if 0
  if (tape->timeBetweenPackets == 0) {
    trap(0x1c00);
  }
  if ((tape->timeBetweenPackets == -1) && (scaledNum == -0x80000000)) {
    trap(0x1800);
  }
#endif

	// 0% = 0,
	// 100% = 0x1000 (4096)
	scaledPacketIdx = scaledNum / tape->timeBetweenPackets;
	packetIdx = (int)scaledPacketIdx >> 0xc;
	lerp4096 = scaledPacketIdx & 0xfff;

	if (tape->numPacketsInArray <= packetIdx)
	{
		packetIdx = tape->numPacketsInArray - 1;
		lerp4096 = 0;
	}

	// Ptrs to current and next packets for better readability
	struct GhostPacket *currPacket = &tape->packets[packetIdx];
	struct GhostPacket *nextPacket = &tape->packets[packetIdx + 1];

	int vel[3];
	vel[0] = (int)nextPacket->pos[0] - (int)currPacket->pos[0];
	vel[1] = (int)nextPacket->pos[1] - (int)currPacket->pos[1];
	vel[2] = (int)nextPacket->pos[2] - (int)currPacket->pos[2];

	inst->matrix.t[0] = currPacket->pos[0] + ((vel[0] * lerp4096) >> 0xC);
	inst->matrix.t[1] = currPacket->pos[1] + ((vel[1] * lerp4096) >> 0xC);
	inst->matrix.t[2] = currPacket->pos[2] + ((vel[2] * lerp4096) >> 0xC);

	// Calculate delta + perform 12-bit wrapping and lerp
	delta = ((int)nextPacket->rot[0] - (int)currPacket->rot[0]) & 0xFFF;
	if (delta > 0x7FF)
		delta -= 0x1000;
	local_rot[0] = currPacket->rot[0] + ((delta * lerp4096) >> 0xC) & 0xFFF;

	delta = ((int)nextPacket->rot[1] - (int)currPacket->rot[1]) & 0xFFF;
	if (delta > 0x7FF)
		delta -= 0x1000;
	local_rot[1] = currPacket->rot[1] + ((delta * lerp4096) >> 0xC) & 0xFFF;

	delta = ((int)nextPacket->rot[2] - (int)currPacket->rot[2]) & 0xFFF;
	if (delta > 0x7FF)
		delta -= 0x1000;
	local_rot[2] = currPacket->rot[2] + ((delta * lerp4096) >> 0xC) & 0xFFF;

	// Retail converts the interpolated rotation into the instance matrix.
	ConvertRotToMatrix(&inst->matrix, local_rot);

	d->posCurr.x = inst->matrix.t[0] << 8;
	d->posCurr.y = inst->matrix.t[1] << 8;
	d->posCurr.z = inst->matrix.t[2] << 8;

	d->rotCurr.x = local_rot[0];
	d->rotCurr.y = local_rot[1];
	d->rotCurr.z = local_rot[2];

	u8 *buffer = tape->packets[packetIdx].bufferPacket;

	while (tape->packetID < packetIdx)
	{
		if (tape->ptrEnd <= (void *)buffer)
			break;

		u8 opcode = buffer[0];

		if (4 < (opcode + 0x80 & 0xFF))
		{
			buffer += 5; // Skip velocity data, assumed to be 5 bytes
			tape->packetID++;
		}

		else
		{
			switch (opcode)
			{
			case 0x80:         // Position and Rotation
				buffer += 0xB; // Skip 11 bytes of position and rotation data
				tape->packetID++;
				break;

			case 0x81:
			{ // Animation

				int numAnimFrames = INSTANCE_GetNumAnimFrames(inst, buffer[1]);
				inst->animIndex = (numAnimFrames < 1) ? 0 : buffer[1];

				int maxFrame = INSTANCE_GetNumAnimFrames(inst, inst->animIndex) - 1;
				if (buffer[2] != 0)
				{
					if (buffer[2] < maxFrame)
						inst->animFrame = buffer[2];
					else
						inst->animFrame = maxFrame;
				}
				else if (maxFrame > 0)
				{
					inst->animFrame = maxFrame;
				}
				else
				{
					inst->animFrame = 0;
				}

				buffer += 3;
			}
			break;

			case 0x82: // Boost
				if (gGT->trafficLightsTimer < 1 && ((gGT->gameMode1 & START_OF_RACE) == 0) && (RaceFlag_IsFullyOnScreen() == 0))
				{
					VehFire_Increment(d,
					                  (int)(buffer[1] << 8 | buffer[2]), // endian flip
					                  buffer[3],
					                  (int)(buffer[4] << 8 | buffer[5]) // endian flip
					);
				}
				buffer += 6;
				break;

			case 0x83:                     // Instance Flags
				inst->flags &= 0xFFFFDFFF; // Reset flag
				if (buffer[1] != 0)
				{
					inst->flags |= SPLIT_LINE;
				}
				buffer += 2;
				break;

			case 0x84: // No-Op
				buffer += 1;
				tape->packetID++;
				break;
			}
		}
	}

	if (gGT->trafficLightsTimer < 1)
	{
		tape->timeElapsedInRace += gGT->elapsedTimeMS;
	}
	return;
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80027838-0x80027b88.
void GhostReplay_Init1(void)
{
	int i;
	struct Thread *t;
	struct Instance *inst;
	struct Instance *wakeInst;
	struct Driver *ghostDriver;
	struct Model *model;
	struct Model *wake;
	int timeTrialFlags;

	struct GhostHeader *gh;
	struct GhostTape *tape;
	int charID;
	char *recordBuffer;

	struct GameTracker *gGT = sdata->gGT;

	// This has to run from MainInit_Drivers
	sdata->boolCanSaveGhost = 0;
	sdata->boolGhostsDrawing = 0;

	// only continue if you're in time trial, not main menu, and not cutscene
	if ((gGT->gameMode1 & 0x20022000) != 0x20000)
		return;

	// === Record Buffer ===

	// In the future, this can move to GhostTape_Start, when byte budget allows

	gh = MEMPACK_AllocMem(0x3e00 /*, "ghost record buffer"*/);
	recordBuffer = GHOSTHEADER_GETRECORDBUFFER(gh);
	sdata->GhostRecording.ptrGhost = gh;
	sdata->GhostRecording.ptrStartOffset = &recordBuffer[0];
	sdata->GhostRecording.ptrEndOffset = &recordBuffer[0x3DD4];

	// === Replay Buffer ===
	// 0: human ghost
	// 1: N Tropy / Oxide ghost

	// ALWAYS initialize ghost threads
	// even if gh == 0, or else the text
	// for "Ghost Too Big" will never play
	for (i = 0; i < 2; i++)
	{
		tape = MEMPACK_AllocMem(0x268 /*, "ghost tape"*/);
		sdata->ptrGhostTape[i] = tape;

		// first ghost pointer is a ghost loaded by player
		if (i == 0)
		{
			// assign the ghost you loaded
			gh = sdata->ptrGhostTapePlaying;
		}

		// second ghost pointer is n tropy or oxide
		else
		{
			// Bitwise ORs
			// 0|0|0 (0) - No Ghost Unlocked
			// 1|0|0 (1) - NTropy Ghost Open
			// 1|2|0 (3) - NTropy Ghost Beaten, Oxide Ghost Open
			// 1|2|4 (7) - NOxide Ghost Beaten
			timeTrialFlags = sdata->gameProgress.highScoreTracks[gGT->levelID].timeTrialFlags;

			void **pointers = ST1_GETPOINTERS(gGT->level1->ptrSpawnType1);

			if ((timeTrialFlags & 2) != 0)
			{
				gh = pointers[ST1_NOXIDE];
			}
			else
			{
				gh = pointers[ST1_NTROPY];
			}
		}

		recordBuffer = GHOSTHEADER_GETRECORDBUFFER(gh);

		tape->gh = gh;
		tape->gh_again = gh;
		tape->ptrStart = &recordBuffer[0];
		tape->constDEADC0ED = 0xDEADC0ED;
		tape->ptrEnd = &recordBuffer[gh->size];

		// if n tropy / oxide
		if (i == 1)
		{
			// guaranteed gh != 0, so dont nullptr check
			gGT->timeToBeatInTimeTrial_ForCurrentEvent = gh->timeElapsedInRace;
		}
	}

	for (i = 0; i < 2; i++)
	{
		t = PROC_BirthWithObject(

		    // creation flags
		    SIZE_RELATIVE_POOL_BUCKET(4, NONE, LARGE, GHOST),

		    GhostReplay_ThTick, sdata->s_ghost, 0);

		t->modelIndex = DYNAMIC_GHOST; // ghost
		t->flags |= 0x1000;            // ignore collisions

		// ghost drivers are 0x638 bytes large
		ghostDriver = t->object;
		memset(ghostDriver, 0, 0x638);
		ghostDriver->ghostID = i;
		ghostDriver->driverID = i + 1;
		ghostDriver->ghostBoolInit = 0;
		ghostDriver->ghostTape = sdata->ptrGhostTape[i];

		// characterID and model
		charID = data.characterIDs[i + 1];
		model = VehBirth_GetModelByName(data.MetaDataCharacters[charID].name_Debug);

		inst = INSTANCE_Birth3D(model, model->name, t);
		t->inst = inst;

		// Ptr Model "Wake"
		wake = gGT->modelPtr[STATIC_WAKE];

		// if "Wake" model exists
		if (wake)
		{
			wakeInst = INSTANCE_Birth3D(wake, wake->name, 0);
			ghostDriver->wakeInst = wakeInst;

			if (wakeInst != 0)
			{
				// make invisible, set to anim 1
				wakeInst->flags |= 0x90;
			}
		}

		inst->unk51 = 0xc;
		inst->flags |= 0x4000000;
		ghostDriver->instSelf = inst;
		VehBirth_TireSprites(t);
		VehBirth_SetConsts(ghostDriver);

		ghostDriver->actionsFlagSet |= 0x100000; // AI driver

		// pointer to TrTire, for transparent tires
		ghostDriver->wheelSprites = ICONGROUP_GETICONS(gGT->iconGroup[0xc]);

		// NOTE(aalhendi): GhostReplay_Init2 owns retail activation/tick.
	}

	return;
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80027b88-0x80027df4.
void GhostReplay_Init2(void)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Thread *thread = gGT->threadBuckets[GHOST].thread;
	struct Driver *driver;
	struct GhostTape *tape;
	int ghostID;
	int characterIndex;
	int characterID;
	int timeTrialFlags;
	struct Instance *inst;
	struct Model *model;
	char *name;

	for (; thread != NULL; thread = thread->siblingThread)
	{
		driver = thread->object;
		if (driver == NULL)
			continue;

		tape = driver->ghostTape;
		if (tape->ptrEnd == tape->ptrStart)
			continue;

		ghostID = driver->ghostID;
		if (ghostID == 0)
		{
			if (sdata->boolReplayHumanGhost == 0)
				continue;
		}
		else
		{
			if (ghostID != 1)
				continue;

			timeTrialFlags = sdata->gameProgress.highScoreTracks[gGT->levelID].timeTrialFlags;
			if ((timeTrialFlags & 1) == 0)
				continue;
		}

		tape->timeElapsedInRace = 0;
		tape->timeInPacket32_backup = 0;
		tape->unk20 = 0;
		tape->timeInPacket32 = 0;
		tape->timeInPacket01 = 0;
		tape->ptrCurr = tape->ptrStart;

		sdata->boolGhostsDrawing = 1;
		driver->ghostBoolInit = 1;
		driver->ghostBoolStarted = 0;

		characterIndex = ghostID + 1;
		if (ghostID != 0)
		{
			timeTrialFlags = sdata->gameProgress.highScoreTracks[gGT->levelID].timeTrialFlags;
			if ((timeTrialFlags & 2) != 0)
				characterIndex = ghostID + 2;
		}

		characterID = data.characterIDs[characterIndex];
		model = VehBirth_GetModelByName(data.MetaDataCharacters[characterID].name_Debug);

		driver->wheelSize = 0;
		if (characterID != NITROS_OXIDE)
			driver->wheelSize = 0xccc;

		inst = driver->instSelf;
		name = sdata->s_ghost0;
		if (ghostID != 0)
			name = sdata->s_ghost1;

		INSTANCE_Birth(inst, model, name, inst->thread, 7);
		GhostReplay_ThTick(thread);

		tape->unk2[0] = tape->unk1[0];
		tape->unk2[1] = tape->unk1[1];
		tape->unk2[2] = tape->unk1[2];
		tape->unk4[0] = tape->unk3[0];
		tape->unk4[1] = tape->unk3[1];
		tape->unk4[2] = tape->unk3[2];
		tape->unk20 = 0;
	}
}
