#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80031b50-0x80031bdc.
void LOAD_GlobalModelPtrs_MPK()
{
	struct GameTracker *gGT = sdata->gGT;

	for (int i = 0; i < 3; i++)
	{
		struct Model *m = (struct Model *)data.driverModelExtras[i];

		if (m == NULL)
			continue;

		if (m->id == -1)
			continue;

		gGT->modelPtr[m->id] = m;
	}

	if (sdata->PLYROBJECTLIST != 0)
	{
		LibraryOfModels_Store(gGT, -1, (struct Model **)sdata->PLYROBJECTLIST);
	}
}
