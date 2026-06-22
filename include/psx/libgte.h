/*
 * Derived from REDRIVER2/PsyCross MIT source:
 * externals/PsyCross/include/psx/libgte.h
 * See THIRD_PARTY_NOTICES.md for copyright and license details.
 */

#ifndef LIBGTE_H
#define LIBGTE_H

#include <stdint.h>

typedef struct
{
	short m[3][3]; /* 3x3 rotation matrix */
	int t[3];      /* transfer vector */
} MATRIX;

typedef struct
{ /* int  word type 3D vector */
	int vx, vy;
	int vz, pad;
} VECTOR;

typedef struct
{ /* short word type 3D vector */
	short vx, vy;
	short vz, pad;
} SVECTOR;

typedef struct
{ /* color type vector */
	unsigned char r, g, b, cd;
} CVECTOR;

typedef struct
{ /* 2D short vector */
	short vx, vy;
} DVECTOR;


extern void InitGeom();
extern void SetGeomOffset(int ofx, int ofy);
extern void SetGeomScreen(int h);

extern void SetRotMatrix(MATRIX *m);
extern void SetLightMatrix(MATRIX *m);
extern void SetColorMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern void PushMatrix();
extern void PopMatrix();
VECTOR *ApplyMatrix(MATRIX *m, SVECTOR *v0, VECTOR *v1);
VECTOR *ApplyRotMatrix(SVECTOR *v0, VECTOR *v1);
VECTOR *ApplyRotMatrixLV(VECTOR *v0, VECTOR *v1);
SVECTOR *ApplyMatrixSV(MATRIX *m, SVECTOR *v0, SVECTOR *v1);
VECTOR *ApplyMatrixLV(MATRIX *m, VECTOR *v0, VECTOR *v1);
extern void RotTrans(SVECTOR *v0, VECTOR *v1, int32_t *flag);
extern void RotTransSV(SVECTOR *v0, SVECTOR *v1, int32_t *flag);
extern int RotTransPers(SVECTOR *v0, int32_t *sxy, int32_t *p, int32_t *flag);
extern int RotTransPers3(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, int32_t *sxy0, int32_t *sxy1, int32_t *sxy2, int32_t *p, int32_t *flag);
extern int RotTransPers4(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, SVECTOR *v3, int32_t *sxy0, int32_t *sxy1, int32_t *sxy2, int32_t *sxy3, int32_t *p,
                         int32_t *flag);
extern void NormalColor(SVECTOR *v0, CVECTOR *v1);
extern void NormalColor3(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, CVECTOR *v3, CVECTOR *v4, CVECTOR *v5);
extern void NormalColorDpq(SVECTOR *v0, CVECTOR *v1, int p, CVECTOR *v2);
extern void NormalColorCol(SVECTOR *v0, CVECTOR *v1, CVECTOR *v2);
extern void NormalColorCol3(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, CVECTOR *v3, CVECTOR *v4, CVECTOR *v5, CVECTOR *v6);
extern void LocalLight(SVECTOR *v0, VECTOR *v1);
extern int RotAverageNclip4(SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, SVECTOR *v3, int32_t *sxy0, int32_t *sxy1, int32_t *sxy2, int32_t *sxy3, int32_t *p,
                            int32_t *otz, int32_t *flag);
extern MATRIX *MulMatrix0(MATRIX *m0, MATRIX *m1, MATRIX *m2);
extern MATRIX *MulMatrix(MATRIX *m0, MATRIX *m1);
extern MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1);
extern void SetBackColor(int rbk, int gbk, int bbk);
extern void SetFarColor(int rfc, int gfc, int bfc);
extern MATRIX *RotMatrix(SVECTOR *r, MATRIX *m);
extern MATRIX *RotMatrixYXZ(SVECTOR *r, MATRIX *m);
extern MATRIX *RotMatrixZYX_gte(SVECTOR *r, MATRIX *m);
extern MATRIX *RotMatrixX(int r, MATRIX *m);
extern MATRIX *RotMatrixY(int r, MATRIX *m);
extern MATRIX *RotMatrixZ(int r, MATRIX *m);
extern MATRIX *TransMatrix(MATRIX *m, VECTOR *v);
extern MATRIX *CompMatrix(MATRIX *m0, MATRIX *m1, MATRIX *m2);
extern MATRIX *ScaleMatrix(MATRIX *m, VECTOR *v);
extern MATRIX *MulRotMatrix(MATRIX *m0);
extern void ColorDpq(VECTOR *v0, CVECTOR *v1, int p, CVECTOR *v2);
extern void ColorCol(VECTOR *v0, CVECTOR *v1, CVECTOR *v2);
extern int NormalClip(int sxy0, int sxy1, int sxy2);
extern void SetDQA(int iDQA);
extern void SetDQB(int iDQB);
extern void SetFogNear(int a, int h);
extern void SetFogNearFar(int a, int b, int h);
extern int SquareRoot0(int a);

extern int rsin(int a);
extern int rcos(int a);
extern int ratan2(int y, int x);


#endif
