#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800644d0-0x80064568
void VehPhysProc_SpinStop_Init(struct Thread *t, struct Driver *d)
{
	d->funcPtrs[0] = NULL;
	d->funcPtrs[1] = VehPhysProc_SpinStop_Update;
	d->funcPtrs[2] = VehPhysProc_SpinStop_PhysLinear;
	d->funcPtrs[3] = DECOMP_VehPhysProc_Driving_Audio;
	d->funcPtrs[4] = VehPhysProc_SpinStop_PhysAngular;
	d->funcPtrs[5] = VehPhysForce_OnApplyForces;

#if !defined(REBUILD_PS1) || defined(CTR_NATIVE)
	d->funcPtrs[6] = COLL_MOVED_PlayerSearch;
	d->funcPtrs[7] = VehPhysForce_CollideDrivers;
#else
	// TODO(aalhendi): Restore retail collision slots for non-native PS1 rebuild
	// once these paths are source-backed there.
	d->funcPtrs[6] = NULL;
	d->funcPtrs[7] = NULL;
#endif

	d->funcPtrs[8] = COLL_FIXED_PlayerSearch;
	d->funcPtrs[9] = VehPhysGeneral_JumpAndFriction;
	d->funcPtrs[10] = VehPhysForce_TranslateMatrix;
	d->funcPtrs[11] = VehPhysProc_SpinStop_Animate;
	d->funcPtrs[12] = VehEmitter_DriverMain;
}

void DECOMP_VehPhysProc_SpinStop_Init(struct Thread *t, struct Driver *d)
{
	VehPhysProc_SpinStop_Init(t, d);
}
