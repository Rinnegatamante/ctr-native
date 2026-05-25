#include <common.h>

void GhostTape_Destroy()
{
	if (sdata->ptrGhostTapePlaying != 0)
	{
		sdata->ptrGhostTapePlaying = 0;
		MEMPACK_ClearHighMem();
	}
}
