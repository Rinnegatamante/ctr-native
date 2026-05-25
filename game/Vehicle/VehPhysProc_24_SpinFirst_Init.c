#include <common.h>

void COLL_FIXED_PlayerSearch();

void *PlayerSpinningFuncTable[0xD] = {VehPhysProc_SpinFirst_InitSetUpdate,
                                      0,
                                      VehPhysProc_SpinFirst_PhysLinear,
                                      VehPhysProc_Driving_Audio,
                                      VehPhysProc_SpinFirst_PhysAngular,
                                      VehPhysForce_OnApplyForces,
                                      COLL_MOVED_PlayerSearch,
                                      VehPhysForce_CollideDrivers,
                                      COLL_FIXED_PlayerSearch,
                                      VehPhysGeneral_JumpAndFriction,
                                      VehPhysForce_TranslateMatrix,
                                      VehFrameProc_Spinning,
                                      VehEmitter_DriverMain};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80063ec0-0x8006402c
void VehPhysProc_SpinFirst_Init(struct Thread *t, struct Driver *d)
{
	int i;
	int feedback;

	d->kartState = KS_SPINNING;

	d->unk_LerpToForwards = 0;
	d->turbo_MeterRoomLeft = 0;

	if (LOAD_IsOpen_RacingOrBattle())
	{
		RB_Player_ModifyWumpa(d, -1);
	}

	Voiceline_RequestPlay(3, data.characterIDs[d->driverID], 0x10);

	// if spinning left
	d->KartStates.Spinning.spinDir = 1;
	d->KartStates.Spinning.driftSpinRate = 300;

	if (d->ampTurnState < 0)
	{
		// if spinning right
		d->KartStates.Spinning.spinDir = -1;
		d->KartStates.Spinning.driftSpinRate = -300;
	}

	if (d->simpTurnState < 1)
	{
		feedback = 0x19;
	}

	else
	{
		feedback = 0x29;
	}

	for (i = 0; i < 0xD; i++)
	{
		d->funcPtrs[i] = PlayerSpinningFuncTable[i];
	}

	GAMEPAD_JogCon1(d, feedback, 0x60);
}
