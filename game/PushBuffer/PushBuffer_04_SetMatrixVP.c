#include <common.h>

#ifdef REBUILD_PC
static char buf[0x400];
#endif

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80042c04-0x80042e50.
void PushBuffer_SetMatrixVP(struct PushBuffer *pb)
{
	// CameraMatrix
	u32 uVar3;
	u32 uVar4;
	u32 uVar5;
	u32 uVar6;
	s16 sVar7;

	u32 view0;
	u32 view4;
	u32 view8;
	u32 viewC;

#ifdef REBUILD_PC
	char *scratchpad;
	scratchpad = &buf[0];
#endif

	// originally used 556 bytes

#ifdef REBUILD_PC
	MATRIX *matrixDST = (MATRIX *)&scratchpad[0x3d4];
#else
	// ordinary PlayStation 1, use scratchpad
	MATRIX *matrixDST = (MATRIX *)0x1f8003d4;
#endif

#ifndef REBUILD_PC
	*(s16 *)0x1f8003f4 = pb->rot[0];
	*(s16 *)0x1f8003f6 = pb->rot[1];
	*(s16 *)0x1f8003f8 = pb->rot[2];
	ConvertRotToMatrix(matrixDST, (s16 *)0x1f8003f4);
#else
	*(s16 *)&scratchpad[0x3f4] = pb->rot[0];
	*(s16 *)&scratchpad[0x3f6] = pb->rot[1];
	*(s16 *)&scratchpad[0x3f8] = pb->rot[2];
	ConvertRotToMatrix(matrixDST, (s16 *)&scratchpad[0x3f4]);
#endif

	s16 negPos[3];

	pb->matrix_Camera.t[0] = pb->pos[0];
	pb->matrix_Camera.t[1] = pb->pos[1];
	pb->matrix_Camera.t[2] = pb->pos[2];

	negPos[0] = -pb->pos[0];
	negPos[1] = -pb->pos[1];
	negPos[2] = -pb->pos[2];

	// load inverted camera position
#ifndef REBUILD_PC
#define gte_ldVXY0(r0) __asm__ volatile("mtc2   %0, $0" : : "r"(r0))
#define gte_ldVZ0(r0)  __asm__ volatile("mtc2   %0, $1" : : "r"(r0))
	gte_ldVXY0(*(int *)&negPos[0]);
	gte_ldVZ0(negPos[2]);
#else
	gte_ldv0(&negPos[0]);
#endif

#ifndef REBUILD_PC

// gte_SetLightMatrix
#define gte_r8(r0)  __asm__ volatile("ctc2   %0, $8" : : "r"(r0))
#define gte_r9(r0)  __asm__ volatile("ctc2   %0, $9" : : "r"(r0))
#define gte_r10(r0) __asm__ volatile("ctc2   %0, $10" : : "r"(r0))
#define gte_r11(r0) __asm__ volatile("ctc2   %0, $11" : : "r"(r0))
#define gte_r12(r0) __asm__ volatile("ctc2   %0, $12" : : "r"(r0))

#endif

	// CameraMatrix
	uVar3 = *(int *)&matrixDST->m[0][0];
	uVar4 = *(int *)&matrixDST->m[0][2];
	uVar5 = *(int *)&matrixDST->m[1][1];
	uVar6 = *(int *)&matrixDST->m[2][0];
	sVar7 = *(s16 *)&matrixDST->m[2][2];

	// CameraMatrix, for shadows, particles, and audio
	*(int *)((int)&pb->matrix_Camera + 0x0) = uVar3;
	*(int *)((int)&pb->matrix_Camera + 0x4) = uVar4;
	*(int *)((int)&pb->matrix_Camera + 0x8) = uVar5;
	*(int *)((int)&pb->matrix_Camera + 0xC) = uVar6;
	*(s16 *)((int)&pb->matrix_Camera + 0x10) = sVar7;

	// transpose the camera matrix
	view0 = uVar3 & 0xffff | uVar4 & 0xffff0000;
	view4 = uVar6 & 0xffff | uVar3 & 0xffff0000;
	view8 = uVar5 & 0xffff | uVar6 & 0xffff0000;
	viewC = uVar4 & 0xffff | uVar5 & 0xffff0000;

	// CameraTranspose, for lightning during Driver Warping effect
	*(int *)((int)&pb->matrix_CameraTranspose + 0x0) = view0;
	*(int *)((int)&pb->matrix_CameraTranspose + 0x4) = view4;
	*(int *)((int)&pb->matrix_CameraTranspose + 0x8) = view8;
	*(int *)((int)&pb->matrix_CameraTranspose + 0xC) = viewC;
	*(s16 *)((int)&pb->matrix_CameraTranspose + 0x10) = sVar7;

	// load transpose camera matrix
	// similar to gte_SetLightMatrix
#ifndef REBUILD_PC
	gte_r8(view0);
	gte_r9(view4);
	gte_r10(view8);
	gte_r11(viewC);
	gte_r12(sVar7);
#else
	gte_SetLightMatrix(&pb->matrix_CameraTranspose);
#endif

	// multiply inverted camera position,
	// by transpose camera matrix
	gte_llv0();

	gte_stlvnl(&pb->matrix_CameraTranspose.t[0]);
	gte_stlvnl(&pb->matrix_ViewProj.t[0]);

	// start with transpose camera matrix
	*(int *)((int)&pb->matrix_ViewProj + 0x0) = view0;
	*(int *)((int)&pb->matrix_ViewProj + 0x4) = view4;
	*(int *)((int)&pb->matrix_ViewProj + 0x8) = view8;
	*(int *)((int)&pb->matrix_ViewProj + 0xC) = viewC;
	*(s16 *)((int)&pb->matrix_ViewProj + 0x10) = sVar7;

	// NTSC:
	// 0x360/0x600 = 9/16 aspect,
	// 9/16 * 512/216 = 4/3

	// PAL:
	// 0x3B0/0x600 = 59/96 aspect,
	// 59/96 * 512/236 = 4/3

	// Do NOT set to 0x480
	// to change 4/3 to 16/9,
	// it will zoom "in" instead of "out"
	// because of stretching Y instead of X

#if BUILD == EurRetail
#define r360 0x3B0
#else
#define r360 0x360
#endif

// constant denomenator
#define r600 0x600

	// scale position
	pb->matrix_ViewProj.t[1] = pb->matrix_ViewProj.t[1] * r360 / r600;

	// scale Y axis (1)
	pb->matrix_ViewProj.m[1][0] = pb->matrix_ViewProj.m[1][0] * r360 / r600;

	// scale Y axis (2)
	pb->matrix_ViewProj.m[1][1] = pb->matrix_ViewProj.m[1][1] * r360 / r600;

	// scale Y axis (3)
	pb->matrix_ViewProj.m[1][2] = pb->matrix_ViewProj.m[1][2] * r360 / r600;

	// store camera matrix,
	// otherwise oxide intro cutscene bugs out,
	// when crash is sleeping on the grassy hill

#ifndef REBUILD_PC
	gte_r8(uVar3);
	gte_r9(uVar4);
	gte_r10(uVar5);
	gte_r11(uVar6);
#else
	gte_SetLightMatrix(&scratchpad[0x3d4]);
#endif

	return;
}
