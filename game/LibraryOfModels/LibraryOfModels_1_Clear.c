#include <common.h>

void LibraryOfModels_Clear(struct GameTracker *gGT)
{
	int i;
	for (i = 0; i < 0xe1; i++)
		gGT->modelPtr[i] = 0;
}
