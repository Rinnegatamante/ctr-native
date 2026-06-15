#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 231 0x800ac1b0-0x800ac220.

int RB_Hazard_HurtDriver(struct Driver *driverVictim, int damageType, struct Driver *driverAttacker, int reason)
{
	struct GameTracker *gGT = sdata->gGT;
	int result = 0;

	if ((driverVictim->actionsFlagSet & ACTION_BOT) == 0)
	{
		result = VehPickState_NewState(driverVictim, damageType, driverAttacker, reason);
	}
	else
	{
		if ((gGT->levelID == OXIDE_STATION) && (gGT->gameMode1 < 0))
			damageType = 1;

		result = (int)BOTS_ChangeState(driverVictim, damageType, driverAttacker, reason);
	}
	return result;
}
