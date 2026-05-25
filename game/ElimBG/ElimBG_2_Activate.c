#include <common.h>

void ElimBG_Activate(struct GameTracker *gGT)
{
	sdata->pause_backup_renderFlags = gGT->renderFlags;
	sdata->pause_backup_hudFlags = gGT->hudFlags;
	sdata->pause_state = 1;
}
