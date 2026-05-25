#include <common.h>

void MEMPACK_ClearHighMem()
{
	// reset end-side allocation
	sdata->PtrMempack->lastFreeByte = sdata->PtrMempack->endOfAllocator;
}
