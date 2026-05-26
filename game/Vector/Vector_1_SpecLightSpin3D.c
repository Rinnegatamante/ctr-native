#include <common.h>

static void Vector_SpecLightSpin3D_LightMatrixMul(MATRIX *matrix, const SVec3 *input, SVec3 *output)
{
	VECTOR mac;

	gte_SetLightMatrix(matrix);
	gte_ldv0((SVECTOR *)input);
	gte_llv0();
	gte_stlvnl(&mac);

	output->x = (s16)mac.vx;
	output->y = (s16)mac.vy;
	output->z = (s16)mac.vz;
}

void Vector_SpecLightSpin3D(struct Instance *inst, s16 *rot, s16 *lightDir)
{
	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005741c-0x800576b8.
	MATRIX rotMatrix;
	SVec3 light = {.x = lightDir[0], .y = lightDir[1], .z = lightDir[2]};
	struct GameTracker *gGT = sdata->gGT;
	struct InstDrawPerPlayer *idpp = INST_GETIDPP(inst);

	ConvertRotToMatrix_Transpose(&rotMatrix, rot);

	for (int i = 0; i < gGT->numPlyrCurrGame; i++)
	{
		struct PushBuffer *pb = &gGT->pushBuffer[i];
		SVec3 lightCamera;
		SVec3 lightLocal;
		SVec3 view;
		SVec3 viewLocal;
		SVec3 halfVector;

		Vector_SpecLightSpin3D_LightMatrixMul(&pb->matrix_Camera, &light, &lightCamera);
		Vector_SpecLightSpin3D_LightMatrixMul(&rotMatrix, &lightCamera, &lightLocal);

		inst->unk53 = (char)lightLocal.x;
		inst->reflectionRGBA = (u16)lightLocal.z;

		view.x = inst->matrix.t[0] - pb->pos[0];
		view.y = inst->matrix.t[1] - pb->pos[1];
		view.z = inst->matrix.t[2] - pb->pos[2];
		MATH_VectorNormalize(&view);
		Vector_SpecLightSpin3D_LightMatrixMul(&rotMatrix, &view, &viewLocal);

		halfVector.x = lightLocal.x + viewLocal.x;
		halfVector.y = lightLocal.y + viewLocal.y;
		halfVector.z = lightLocal.z + viewLocal.z;
		MATH_VectorNormalize(&halfVector);

		idpp[i].specLight[0] = halfVector.x;
		idpp[i].specLight[1] = halfVector.y;
		idpp[i].specLight[2] = halfVector.z;
	}
}
