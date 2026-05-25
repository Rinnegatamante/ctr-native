#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002406c-0x800242b8.
void DotLights_Video(struct GameTracker *gGT, int red1, int red2, int red3, int green, int posY)
{
	struct Icon *icon;
	struct PushBuffer *pb;
	int iconState[4];
	int newPosX;
	int newPosY;
	int playerIndex;
	int lightIndex;
	int scale;
	int sizeX;

	if (gGT->numPlyrCurrGame == 0)
		return;

	iconState[0] = red1;
	iconState[1] = red2;
	iconState[2] = red3;
	iconState[3] = green;

	for (playerIndex = 0; playerIndex < gGT->numPlyrCurrGame; playerIndex++)
	{
		pb = &gGT->pushBuffer[playerIndex];

		scale = FP(0.5);
		if (gGT->numPlyrCurrGame == 1)
			scale = FP(1.0);
		else if (gGT->numPlyrCurrGame == 2)
			scale = 0xaaa;

		icon = gGT->trafficLightIcon[0];
		sizeX = FP_Mult(icon->texLayout.u1 - icon->texLayout.u0, scale);

		newPosX = (pb->rect.w - (sizeX * 4)) / 2;
		newPosY = FP_Mult(pb->rect.h / 3, posY) - FP_Mult(icon->texLayout.v2 - icon->texLayout.v0, scale);

		for (lightIndex = 0; lightIndex < 4; lightIndex++)
		{
			DecalHUD_DrawPolyFT4(gGT->trafficLightIcon[iconState[lightIndex] + (2 * (lightIndex == 3))], newPosX + (sizeX * lightIndex), newPosY,
			                     &gGT->backBuffer->primMem, pb->ptrOT, 0, scale);
		}
	}
}
