#include <common.h>

void MainKillGame_StopCTR(void)
{
	DrawSyncCallback(0);
	StopCallback();

#ifndef REBUILD_PC
	MEMCARD_CloseCard();
#endif

	PadStopCom();
	ResetGraph(3);
	VSyncCallback(0);

	Timer_Destroy();
}
