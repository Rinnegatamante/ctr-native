#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80045650-0x8004568c.
void RECTMENU_DrawOuterRect_HighLevel(RECT *r, Color color, s16 param_3, u_long *otMem)
{
	RECTMENU_DrawOuterRect_LowLevel(r, 3, 2, color, param_3, otMem);
	return;
}
