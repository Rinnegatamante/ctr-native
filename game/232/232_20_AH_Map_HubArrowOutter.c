#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b1150-0x800b14f4.
void AH_Map_HubArrowOutter(void *hubPtrs, int arrowIndex, int posX, int posY, int inputAngle, int type)
{
	struct GameTracker *gGT;
	gGT = sdata->gGT;

	arrowIndex = (s16)arrowIndex;
	type = (s16)type;

	posX += D232.hubArrowXY_Inner[2 * type + 0];
	posY += D232.hubArrowXY_Inner[2 * type + 1];

	int timer = gGT->timer >> 0;

	int var14;
	int var15;
	int var8;

	var15 = 0x40;
	if ((timer & 1) != 0)
		var15 = 0xe0;

	if (type == 0)
	{
		var14 = var15;
		var8 = 0x200;
	}

	else if (type == 1)
	{
		var14 = 0xff;
		var8 = 0x555;

		posX += D232.hubArrowXY_Outter[2 * (((inputAngle >> 0x8) & 0xc) >> 2) + 0];
		posY += D232.hubArrowXY_Outter[2 * (((inputAngle >> 0x8) & 0xc) >> 2) + 1];
	}

	else
	{
		var14 = var15;
		var8 = 0x199;
		inputAngle ^= 0x800;
	}

	inputAngle = (s16)inputAngle;

	for (int iVar10 = 0; iVar10 < 3; iVar10++)
	{
		u32 var5 = (~(timer + (int)arrowIndex * 0xc) & 0x3f) + (2 - (int)(s16)iVar10) * -6;

		if (var5 >= 0xc)
			continue;

		int iVar16 = ((var5 * 0x2aa + 0x1000) * 0x10000) >> 0x1a;

		int bVar1 = 1;

		int shiftToggle = 1;

		int iVar6 = 0;
		int iVar9 = 0;

		for (int iVar13 = 0; iVar13 < var8 + 0xfff; iVar13 += var8)
		{
			if (type != 2)
				shiftToggle = 0;

			int angle = iVar13 + inputAngle;

			int sin = MATH_Sin(angle);
			int cos = MATH_Cos(angle);

			int iVar4 = (shiftToggle & 1) + 0xc;

			sin = posX + ((((iVar16 << 3) / 5) * sin) >> iVar4);
			cos = posY - ((((iVar16)) * cos) >> iVar4);

			if (!bVar1)
			{
				CTR_Box_DrawWirePrims((Point){iVar9, iVar6}, (Point){sin, cos}, MakeColor(var14, var15, 0xff), (void *)gGT->pushBuffer_UI.ptrOT);
			}

			bVar1 = 0;
			iVar9 = sin;
			iVar6 = cos;
			shiftToggle++;
		}
	}
}
