#include <common.h>

int MEMPACK_GetFreeBytes()
{
	struct Mempack *ptrMempack;

	// Get the pointer to the memory allocation system
	ptrMempack = sdata->PtrMempack;

	// subtract last byte from first byte, to return size
	return (u32)ptrMempack->lastFreeByte - (u32)ptrMempack->firstFreeByte;
}
