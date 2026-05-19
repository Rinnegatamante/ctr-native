#include <common.h>

void Vector_SpecLightSpin3D(struct Instance *inst, short *rot, short *lightDir);

void CS_Podium_Prize_Spin(struct Instance *inst, short *prize)
{
	struct GamepadSystem *gGS;
	short prevAngle;
	int ratio;
	u_int angle;
	short lightDir[3];

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
		short sine1 = DECOMP_MATH_Sin(angle);
		short cos1 = DECOMP_MATH_Cos(angle);

		lightDir[1] = cos1;
		(void)lightDir[1];

		ratio = (prize[0x10] & 0xfff) - 0x800;
		if (ratio < 0)
			ratio = -ratio;

		angle = prize[0xd] + (((prize[0xf] - prize[0xd]) * ratio) >> 11);

		short sine2 = DECOMP_MATH_Sin(angle);
		short cos2 = DECOMP_MATH_Cos(angle);

		lightDir[0] = (sine1 * cos2) >> 12;
		lightDir[2] = (sine1 * sine2) >> 12;
	}

	Vector_SpecLightSpin3D(inst, &prize[4], lightDir);
}
