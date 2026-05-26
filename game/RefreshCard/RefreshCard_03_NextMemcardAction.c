#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80046b1c-0x80046b60.
void RefreshCard_NextMemcardAction(int slot, int action, char *fileName, char *fileIconHeader, struct GhostHeader *ptrGhostHeader, int fileSize)
{
	sdata->frame4_memcardAction = action;
	sdata->frame2_memcardAction = action;
	sdata->frame4_memcardSlot = slot;
	sdata->frame2_memcardSlot = slot;
	sdata->ghostProfile_fileName = fileName;
	sdata->ghostProfile_fileIconHeader = fileIconHeader;
	sdata->ghostProfile_ptrGhostHeader = ptrGhostHeader;
	sdata->ghostProfile_size3E00 = fileSize;
	sdata->memcardUnk1 &= ~8;
}
