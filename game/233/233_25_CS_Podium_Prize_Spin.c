#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af7c0-0x800af994
void CS_Podium_Prize_Spin(struct Instance *inst, s16 *prize)
{
	struct GamepadSystem *gGS;
	u32 trigApprox;
	s16 prevAngle;
	int ratio;
	u32 angle;
	SVec3 lightDir;

	prize[5] += 100;
	ConvertRotToMatrix(&inst->matrix, &prize[4]);

	gGS = sdata->gGamepads;

	if ((inst->flags & USE_SPECULAR_LIGHT) == 0)
		return;

	prevAngle = prize[0x10];
	prize[0x10] = prevAngle + 0x3f;

	if ((gGS->gamepad[1].buttonsHeldCurrFrame & BTN_L1) != 0)
		prize[0x10] = prevAngle;

	ratio = (prize[0x10] & 0xfff) - 0x800;
	if (ratio < 0)
		ratio = -ratio;

	angle = prize[0xc] + (((prize[0xe] - prize[0xc]) * ratio) >> 11);

	{
		s16 sine1;
		s16 cos1;

		trigApprox = *(u32 *)&data.trigApprox[angle & 0x3ff];
		if ((angle & 0x400) == 0)
		{
			cos1 = (s16)(trigApprox >> 16);
			sine1 = (s16)trigApprox;
		}
		else
		{
			cos1 = -(s16)trigApprox;
			sine1 = (s16)(trigApprox >> 16);
		}
		if ((angle & 0x800) != 0)
		{
			cos1 = -cos1;
			sine1 = -sine1;
		}
		lightDir.y = cos1;

		ratio = (prize[0x10] & 0xfff) - 0x800;
		if (ratio < 0)
			ratio = -ratio;

		angle = prize[0xd] + (((prize[0xf] - prize[0xd]) * ratio) >> 11);

		s16 sine2;
		s16 cos2;

		trigApprox = *(u32 *)&data.trigApprox[angle & 0x3ff];
		if ((angle & 0x400) == 0)
		{
			cos2 = (s16)(trigApprox >> 16);
			sine2 = (s16)trigApprox;
		}
		else
		{
			cos2 = -(s16)trigApprox;
			sine2 = (s16)(trigApprox >> 16);
		}
		if ((angle & 0x800) != 0)
		{
			cos2 = -cos2;
			sine2 = -sine2;
		}
		lightDir.x = (sine1 * cos2) >> 12;
		lightDir.z = (sine1 * sine2) >> 12;
	}

	Vector_SpecLightSpin3D(inst, &prize[4], &lightDir);
}
