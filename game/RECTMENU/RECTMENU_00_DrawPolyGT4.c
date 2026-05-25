#include <common.h>

void DECOMP_RECTMENU_DrawPolyGT4(struct Icon *icon, s16 posX, s16 posY, struct PrimMem *primMem, u_long *ot, u32 color0, u32 color1, u32 color2, u32 color3,
                                 char transparency, s16 scale)
{
	if (!icon)
		return;

	DecalHUD_DrawPolyGT4(icon, posX, posY, primMem, ot, color0, color1, color2, color3, transparency, scale);
}
