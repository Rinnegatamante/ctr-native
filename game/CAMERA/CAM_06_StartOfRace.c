#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80018d20-0x80018d9c
void CAM_StartOfRace(struct CameraDC *cDC)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Level *level1 = gGT->level1;

	// if fly-in camera data exists and there is only one screen
	int hasFlyInCamera = (2 < level1->cnt_restart_points);

	if (hasFlyInCamera)
	{
		int flyInData = (int)level1->ptr_restart_points;
		cDC->unk94 = 0;
		cDC->unk8C = 0;

		// make transition to driver last one second
		cDC->frameCounterTransition = 0x1E;
		cDC->desiredRot[6] = 0;

		// when camera reaches player, be zoomed in
		cDC->cameraMode = 0;
		cDC->unk88 = (void *)(flyInData + 0x18);

		// if 1 or less screens
		// set fly-in to last 165 frames, (5.25 seconds)
		cDC->unk8E = 0xA5;
		if (gGT->numPlyrCurrGame > 1)
		{
			// set animation to last one frame
			cDC->unk8E = 1;
		}
	}

	cDC->cameraMode = 0;

	// remove bit & 0x4 (battle end of race) and 0x1000 (idk)
	cDC->flags &= 0xFFFFEFFB;
	return;
}
