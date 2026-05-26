#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80064c38-0x80064f94.
// boolPlaySound only gates sound when refreshing an existing mask object.
struct MaskHeadWeapon *VehPickupItem_MaskUseWeapon(struct Driver *driver, int boolPlaySound)

{
	struct Thread *currThread;
	struct MaskHeadWeapon *maskObj;
	struct Model *modelPtr;
	struct Thread *t;
	struct GameTracker *gGT;
	struct Instance *instance;
	int soundID;

	gGT = sdata->gGT;

	if ((LOAD_IsOpen_RacingOrBattle() == 0) || ((gGT->gameMode1 & ADVENTURE_ARENA) != 0))
	{
		// no mask object in adv arena
		maskObj = NULL;
		return maskObj;
	}

	t = driver->instSelf->thread;

	// check for existing mask
	for (currThread = t->childThread; currThread != 0; currThread = currThread->siblingThread)
	{
		// if thread->modelIndex is NOT Aku or Uka
		if ((u32)(currThread->modelIndex - STATIC_AKUAKU) >= 2)
			continue;

		currThread->funcThTick = RB_MaskWeapon_ThTick;

		maskObj = currThread->object;
		maskObj->duration = (driver->numWumpas < 10) ? 0x1e00 : 0x2d00;

		if (
		    // If this is human and not AI
		    ((driver->actionsFlagSet & 0x100000) == 0) &&

		    (boolPlaySound != 0))
		{
			// 0x3a: uka model
			// 0x39: aku model

			// 0x54: uka sound
			// 0x53: aku model

			soundID = currThread->modelIndex + 0x1A;
			OtherFX_Play_Echo(soundID, 1, driver->actionsFlagSet & 0x10000);
		}

		// un-kill thread
		currThread->flags &= ~(0x800);

		// return object attached to thread
		return (struct MaskHeadWeapon *)currThread->object;
	}

	int boolGoodGuy = VehPickupItem_MaskBoolGoodGuy(driver);

	int modelID = STATIC_UKAUKA - boolGoodGuy;

	// 0x3a: uka head model idx in modelPtr array
	instance = INSTANCE_BirthWithThread(modelID, sdata->s_doctor1, SMALL, OTHER, RB_MaskWeapon_ThTick, sizeof(struct MaskHeadWeapon), t);

	soundID = modelID + 0x1A;


	if (
	    // If this is human and not AI
	    ((driver->actionsFlagSet & 0x100000) == 0) &&

	    (OtherFX_Play_Echo(soundID, 1, driver->actionsFlagSet & 0x10000),

	     1 < (u32)(driver->kartState - 4)))
	{
		if (boolGoodGuy == 0)
		{
			gGT->gameMode1 &= ~(AKU_SONG);
			gGT->gameMode1 |= UKA_SONG;
		}

		else
		{
			gGT->gameMode1 &= ~(UKA_SONG);
			gGT->gameMode1 |= AKU_SONG;
		}
	}

	// 0x3a: uka model
	// 0x39: aku model

	// 0x40: uka beam
	// 0x3E: aku beam

	modelPtr = gGT->modelPtr[STATIC_AKUBEAM + ((modelID - STATIC_AKUAKU) * 2)];

	t = instance->thread;

	maskObj = (struct MaskHeadWeapon *)t->object;

// NOTE(aalhendi): Native keeps this model lookup string host-side; PS1 uses
// the retail RDATA symbol.
#ifdef CTR_NATIVE
	maskObj->maskBeamInst = INSTANCE_Birth3D(modelPtr, "akubeam1", t);
#else
	maskObj->maskBeamInst = INSTANCE_Birth3D(modelPtr, rdata.s_akubeam1, t);
#endif

	t->funcThDestroy = PROC_DestroyInstance;

	t->flags |= 0x1000;                   // disable collision
	instance->flags |= 0x80;              // make mask head invisible
	maskObj->maskBeamInst->flags |= 0x80; // make mask beam invisible
	maskObj->duration = (driver->numWumpas > 9) ? 0x2d00 : 0x1e00;
	maskObj->rot[0] = 0x40;  // rotX
	maskObj->rot[1] = 0;     // rotY
	maskObj->rot[2] = 0;     // rotZ
	maskObj->scale = 0x1000; // scale

	return maskObj;
}
