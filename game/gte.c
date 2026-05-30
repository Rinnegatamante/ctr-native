#include <ctr_gte.h>

void RotateVector(SVec3 *out, const SVec3 *in)
{
#ifndef CTR_NATIVE
	gte_ldv0(in);
	gte_rtv0();
	gte_stlvnl(0x1f800000);

	out->x = *(s16 *)0x1f800000;
	out->y = *(s16 *)0x1f800004;
	out->z = *(s16 *)0x1f800008;
#endif
}

void MulMatrixVec(Vec3 *out, const Matrix *m, const SVec3 *in)
{
#ifndef CTR_NATIVE
	gte_SetColorMatrix(m);
	gte_ldv0(in);
	gte_lcv0();
	gte_stlvnl(out);
#endif
}
