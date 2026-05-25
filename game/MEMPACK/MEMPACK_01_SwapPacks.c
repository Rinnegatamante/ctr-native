#include <common.h>

void MEMPACK_SwapPacks(int index)
{
	sdata->PtrMempack = &sdata->mempack[index];
	return;
}
