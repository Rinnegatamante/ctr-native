#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3f98-0x800b42b4.
void AH_MaskHint_SetAnim(int scale)
{
	MATRIX *m;
	struct GameTracker *gGT = sdata->gGT;
	struct PushBuffer *pb = &gGT->pushBuffer[0];

	m = &pb->matrix_Camera;
	gte_SetRotMatrix(m);
	gte_SetTransMatrix(m);

	gte_ldv0(&D232.maskOffsetPos[0]);
	gte_rt();

	int posEndINT[3];
	SVec3 posEnd;

	gte_stlvnl(&posEndINT[0]);

	posEnd.x = posEndINT[0];
	posEnd.y = posEndINT[1];
	posEnd.z = posEndINT[2];

	SVec3 rotEnd;
	rotEnd.x = pb->rot.x - D232.maskOffsetRot[0];
	rotEnd.y = pb->rot.y + D232.maskOffsetRot[1];
	rotEnd.z = pb->rot.z - D232.maskOffsetRot[2];

	SVec3 posCurr;
	SVec3 rotCurr;
	CAM_ProcessTransition(&posCurr, &rotCurr, (SVec3 *)&D232.maskCamPosStart[0], (SVec3 *)&D232.maskCamRotStart[0], &posEnd, &rotEnd, scale);

	int rot = 0x1000;
	if (D232.maskSpawnFrame - 20 < D232.maskFrameCurr)
	{
		rot = ((D232.maskSpawnFrame - D232.maskFrameCurr) * rot) / 20;
	}

	// 4096->50
	rot = (rot * 50) >> 0xc;

	int angle = (scale << 0xf) >> 0xc;
	D232.maskAngle = angle;

	int sin = MATH_Sin(angle);
	int cos = MATH_Cos(angle);

	struct Instance *mhInst = sdata->instMaskHints3D;
	posCurr.x += (s16)((sin * rot) >> 0xc);
	posCurr.z += (s16)((cos * rot) >> 0xc);

	rotCurr.y += angle;
	ConvertRotToMatrix(&mhInst->matrix, &rotCurr);

	((struct MaskHint *)mhInst->thread->object)->scale = scale * 4 - 1;

	angle = (sdata->frameCounter + gGT->timer) * 0x20;
	sin = MATH_Sin(angle);
	posCurr.y += (s16)(((sin << 4) >> 0xc) * scale >> 0xc);

	mhInst->matrix.t[0] = posCurr.x;
	mhInst->matrix.t[1] = posCurr.y;
	mhInst->matrix.t[2] = posCurr.z;
}
