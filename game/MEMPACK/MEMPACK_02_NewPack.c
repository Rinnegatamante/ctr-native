#include <common.h>

void MEMPACK_NewPack(void *start, int size)
{
	struct Mempack *ptrMempack;
	ptrMempack = sdata->PtrMempack;

	ptrMempack->start = start;
	ptrMempack->firstFreeByte = start;

	void *end = (void *)((u32)start + size);
	ptrMempack->lastFreeByte = end;
	ptrMempack->endOfAllocator = end;

	ptrMempack->numBookmarks = 0;

	return;
}
