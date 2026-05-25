#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac1b0-0x800ac204.

int RB_Hazard_HurtDriver(struct Driver *driverVictim, int damageType, struct Driver *driverAttacker, int reason)
{
	struct GameTracker *gGT = sdata->gGT;
	int result = 0;

	if ((driverVictim->actionsFlagSet & 0x100000) == 0)
	{
		result = VehPickState_NewState(driverVictim, damageType, driverAttacker, reason);
	}
	else
	{
		// decomp attempt 1 had the second condition of this if statement as: ((gGT->gameMode1 & ADVENTURE_BOSS) != 0)
		// currently using ((int)gGT->gameMode1 < 0) bc that's what ghidra says (it might be equivalent idk)
		if ((gGT->levelID == OXIDE_STATION) && (gGT->gameMode1 < 0))
			damageType = 1;

		result = (int)BOTS_ChangeState(driverVictim, damageType, driverAttacker, reason);
	}
	return result;
}
