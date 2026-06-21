#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b351c-0x800b3594.
void AH_HintMenu_MaskPosRot(void)
{
	struct Instance *mask = sdata->instMaskHints3D;

	ConvertRotToMatrix(&mask->matrix, &D232.maskRot);

	// Set position
	mask->matrix.t[0] = D232.maskPos.x;
	mask->matrix.t[1] = D232.maskPos.y;
	mask->matrix.t[2] = D232.maskPos.z;

	// always 0x1000 ???
	((struct MaskHint *)mask->thread->object)->scale = D232.maskScale;

	return;
}
