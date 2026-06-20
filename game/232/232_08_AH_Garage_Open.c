#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae8e0-0x800ae988.
void AH_Garage_Open(struct ScratchpadStruct *sps, void *hitObject)
{
	struct Thread *otherTh = hitObject;
	s16 sound;
	struct Instance *garageInst;
	struct BossGarageDoor *garage;
	struct Thread *garageThread;

	if (otherTh->modelIndex != DYNAMIC_PLAYER)
		return;

	garageThread = sps->Union.ThBuckColl.thread;

	garage = garageThread->object;
	garageInst = garageThread->inst;

	if (
	    // if door is not opening
	    (garage->direction != 1) &&

	    // if door is closed,
	    // if posY is the same as instDef posY
	    (garageInst->matrix.t[1] == garageInst->instDef->pos.y))
	{
		// if you are not in gemstone valley
		// play sound of normal boss door opening
		sound = 0x95;

		// Level ID
		// if you are in Gemstone Valley
		if (sdata->gGT->levelID == GEM_STONE_VALLEY)
		{
			// play sound of oxide door opening
			sound = 0x96;
		}

		// Play sound
		OtherFX_Play(sound, 1);
	}

	// door is now opening
	garage->direction = 1;

	// enable access through a door (disable collision)
	sdata->doorAccessFlags |= 1;
}
