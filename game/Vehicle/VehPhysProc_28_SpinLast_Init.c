#include <common.h>

void COLL_FIXED_PlayerSearch();

void *PlayerLastSpinFuncTable[0xD] = {0,
                                      DECOMP_VehPhysProc_SpinLast_Update,
                                      DECOMP_VehPhysProc_SpinLast_PhysLinear,
                                      DECOMP_VehPhysProc_Driving_Audio,
                                      DECOMP_VehPhysProc_SpinLast_PhysAngular,
                                      VehPhysForce_OnApplyForces,

#ifndef REBUILD_PS1
                                      COLL_MOVED_PlayerSearch,
                                      VehPhysForce_CollideDrivers,
                                      COLL_FIXED_PlayerSearch,
                                      VehPhysGeneral_JumpAndFriction,
                                      VehPhysForce_TranslateMatrix,
                                      VehFrameProc_LastSpin,

                                      VehEmitter_DriverMain

#else
#ifdef CTR_NATIVE
                                      COLL_MOVED_PlayerSearch,
                                      VehPhysForce_CollideDrivers,
#else
                                      NULL,
                                      NULL,
#endif
                                      COLL_FIXED_PlayerSearch,
                                      VehPhysGeneral_JumpAndFriction,
                                      VehPhysForce_TranslateMatrix,
                                      VehFrameProc_LastSpin,
                                      VehEmitter_DriverMain
#endif
};

void DECOMP_VehPhysProc_SpinLast_Init(struct Thread *t, struct Driver *d)
{
	int i;

	for (i = 0; i < 0xD; i++)
	{
		d->funcPtrs[i] = PlayerLastSpinFuncTable[i];
	}
}
