#include <common.h>

static int DotLights_TweenPos(int value)
{
	int quotient = (int)((s64)((s64)value * -0x77777777) >> 0x20);
	return ((quotient + value) >> 9) - (value >> 0x1f);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800242b8-0x80024464.
void DotLights_AudioAndVideo(struct GameTracker *gGT)
{
	int timer = gGT->trafficLightsTimer;
	int red1;
	int red2;
	int red3;
	int green;
	int posY;

	if (timer >= -0x3bf)
	{
		if (timer < 1)
		{
			if (sdata->trafficLightsTimer_prevFrame > 0)
				DECOMP_OtherFX_Play(0x46, 0);

			red1 = 1;
			red2 = 1;
			red3 = 1;
			green = 1;
			posY = DotLights_TweenPos((gGT->trafficLightsTimer + 0x3c0) * 0x1000);
		}
		else if (timer < 0x3c1)
		{
			if (sdata->trafficLightsTimer_prevFrame > 0x3c0)
				DECOMP_OtherFX_Play(0x45, 0);

			red1 = 1;
			red2 = 1;
			red3 = 1;
			green = 0;
			posY = 0x1000;
		}
		else if (timer < 0x781)
		{
			if (sdata->trafficLightsTimer_prevFrame > 0x780)
				DECOMP_OtherFX_Play(0x45, 0);

			red1 = 1;
			red2 = 1;
			red3 = 0;
			green = 0;
			posY = 0x1000;
		}
		else if (timer < 0xb41)
		{
			if (sdata->trafficLightsTimer_prevFrame > 0xb40)
				DECOMP_OtherFX_Play(0x45, 0);

			red1 = 1;
			red2 = 0;
			red3 = 0;
			green = 0;
			posY = 0x1000;
		}
		else
		{
			red1 = 0;
			red2 = 0;
			red3 = 0;
			green = 0;
			posY = DotLights_TweenPos((0xf00 - gGT->trafficLightsTimer) * 0x1000);
		}

		DotLights_Video(gGT, red1, red2, red3, green, posY);
	}

	sdata->trafficLightsTimer_prevFrame = gGT->trafficLightsTimer;
}
