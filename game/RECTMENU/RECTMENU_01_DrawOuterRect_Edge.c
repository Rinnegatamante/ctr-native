#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80044f90-0x80044ff8.
void RECTMENU_DrawOuterRect_Edge(RECT *r, Color color, u32 param_3, u_long *otMem)
{
	param_3 & 0x20 ? CTR_Box_DrawClearBox(r, &color, TRANS_50_DECAL, otMem) : CTR_Box_DrawSolidBox(r, color, otMem);
}
