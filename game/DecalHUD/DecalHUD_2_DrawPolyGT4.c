#include <common.h>

#define EDUCATIONAL_BUG_IF 0

void DecalHUD_DrawPolyGT4(struct Icon *icon, s16 posX, s16 posY, struct PrimMem *primMem, u_long *ot, u32 color0, u32 color1, u32 color2, u32 color3,
                          char transparency, s16 scale)
{
#if BUILD > SepReview
	if (!icon)
		return;
#endif

	// setInt32RGB4 needs to go before addPolyGT4
	// for more information check "include/gpu.h"
	POLY_GT4 *p = (POLY_GT4 *)primMem->curr;
	setInt32RGB4(p, color0, color1, color2, color3);
	addPolyGT4(ot, p);

	u32 width = icon->texLayout.u1 - icon->texLayout.u0;
	u32 height = icon->texLayout.v2 - icon->texLayout.v0;
	u32 bottomY = posY + FP_Mult(height, scale);

#if EDUCATIONAL_BUG_IF
	// The original compiled version of DecalHUD_DrawPolyGT4 has a bug where the Y coordinate of the top
	// right vertex of the primitive (henceforth referred to as y1) can overflow by 1 if the X offset of
	// the primitive (centered on the top left vertex of the primitive, henceforth referred to as x0) is
	// set to a negative value. This is due to an optimization in the code: with x0 being a 32-bit value
	// truncated to 16 bits, the value for the X coordinate of the top right vertex (henceforth referred to
	// as x1), itself the offset of x0 by the width of the texture, is stored in a 32-bit variable; as the
	// function sets the primitive's X and Y coordinate values by dereferencing both of them as a singular
	// 32-bit integer, with x1 and y1's bits being added together using bitwise OR, the bottom 16 bits of
	// x1 are added onto y1's bits, resulting in y1's value being altered by x1's overflow.
	// This also affects the bottom right vertex.

	u32 rightX = (u16)posX + FP_Mult(width, scale);
	setXY4CompilerHack(p, (u16)posX, posY, rightX, posY, (u16)posX, bottomY, rightX, bottomY);
#else
	u32 rightX = posX + FP_Mult(width, scale);
	setXY4(p, posX, posY, rightX, posY, posX, bottomY, rightX, bottomY);
#endif
	setIconUV(p, icon);

	if (transparency)
	{
		setTransparency(p, transparency);
	}

	primMem->curr = p + 1;
}
