#include <common.h>

extern void *PlayerAntiVShiftFuncTable[13];

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80062e94-0x80062f4c.
void VehPhysProc_FreezeVShift_Init(struct Thread *t, struct Driver *d)
{
	d->kartState = KS_ANTIVSHIFT;

	// Turbo meter = full
	d->turbo_MeterRoomLeft = 0;

	// turn off 29th flag of actions flag set (means players dont collide anymore)
	d->actionsFlagSet &= ~(0x10000000);

	for (int i = 0; i < 13; i++)
	{
		d->funcPtrs[i] = PlayerAntiVShiftFuncTable[i];
	}
}

void VehPhysProc_FreezeVShift_Update(struct Thread *t, struct Driver *d);
void VehPhysProc_FreezeVShift_ReverseOneFrame(struct Thread *t, struct Driver *d);

void *PlayerAntiVShiftFuncTable[13] = {NULL,
                                       VehPhysProc_FreezeVShift_Update,
                                       VehPhysProc_Driving_PhysLinear,
                                       VehPhysProc_Driving_Audio,
                                       VehPhysGeneral_PhysAngular,
                                       VehPhysForce_OnApplyForces,
                                       COLL_MOVED_PlayerSearch,
                                       VehPhysForce_CollideDrivers,
                                       COLL_FIXED_PlayerSearch,
                                       VehPhysProc_FreezeVShift_ReverseOneFrame,
                                       VehPhysForce_TranslateMatrix,
                                       VehFrameProc_Driving,
                                       VehEmitter_DriverMain};
