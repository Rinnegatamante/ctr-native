#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80027e90-0x80027f20
void GhostTape_End(void)
{
	struct Driver *d;
	struct GhostHeader *gh;
	struct GameTracker *gGT = sdata->gGT;

	// quit, if ghost cant be saved
	if (sdata->boolCanSaveGhost == 0)
		return;

	// dont save ghost twice
	sdata->boolCanSaveGhost = 0;

	// Write the last chunk of ghost data
	GhostTape_WriteMoves(1);

	d = gGT->drivers[0];
	gh = sdata->GhostRecording.ptrGhost;

	gh->ySpeed = d->ySpeed;
	gh->speedApprox = d->speedApprox;
	gh->timeElapsedInRace = d->timeElapsedInRace;
	gh->size = (u32)sdata->GhostRecording.ptrCurrOffset - (u32)sdata->GhostRecording.ptrStartOffset;
}
