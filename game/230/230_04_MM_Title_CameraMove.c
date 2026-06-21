#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ac1f0-0x800ac350.
void MM_Title_CameraMove(struct Title *title, int frameIndex)
{
	int result;
	s16 *posRot;
	struct GameTracker *gGT;

	// after frame 0xe6, make the intro models transition from the center
	// of the screen, to the left of the screen, over the course of 15 frames
	result = RaceFlag_MoveModels(D230.timerInTitle - 0xe6, 0xF);

	gGT = sdata->gGT;

	posRot = &D230.ptrIntroCam[frameIndex * 6];

	for (int i = 0; i < 3; i++)
	{
		// position XYZ
		gGT->pushBuffer[0].pos.v[i] = title->cameraPosOffset.v[i] + posRot[i] + (s16)((D230.titleCameraPos.v[i] * result) >> 0xc);

		// rotation XYZ
		gGT->pushBuffer[0].rot.v[i] = posRot[3 + i] + (s16)((D230.titleCameraRot.v[i] * result) >> 0xc);
	}
}
