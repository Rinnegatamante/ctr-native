#include <common.h>

void ElimBG_Deactivate(struct GameTracker *gGT)
{
	// it's written this way for bytebudget reasons.
	u8 backup = (u8)sdata->pause_backup_hudFlags;

	// if game is paused
	if (sdata->pause_state != 0)
	{
		// if game is not paused
		sdata->pause_state = 3;

		gGT->renderFlags = (gGT->renderFlags & 0x1000) | (sdata->pause_backup_renderFlags & 0xffffefff);

		gGT->hudFlags = backup;
	}
}
