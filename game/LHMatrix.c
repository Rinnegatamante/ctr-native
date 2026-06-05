#include <common.h>


/// @brief Link Hierarchal Matrix (like weapon relative to kart)
/// @param pDst - destination instance
/// @param pSrc - source instance
/// @param transVec - transform vector (x,y,z)
/// 0x800313c8
// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800313c8-0x8003147c.
void LHMatrix_Parent(struct Instance *pDst, struct Instance *pSrc, SVECTOR *transVec)
{
	memcpy(&pDst->matrix, &pSrc->matrix, sizeof(pSrc->matrix));
	SetRotMatrix(&pDst->matrix);
	SetTransMatrix(&pDst->matrix);
	gte_ldv0(transVec);
	gte_rt();
	gte_stlvnl(&pDst->matrix.t);
}
