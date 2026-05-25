#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800b42b0-0x800b4334.
void MM_ResetAllMenus(void)
{
	for (int i = 0; i < 9; i++)
	{
		struct RectMenu *menu = D230.arrayMenuPtrs[i];

// NOTE(aalhendi): Retail resets one menu per array slot; native walks chained
// menus because overlay 230 data is not reloaded.
#ifdef CTR_NATIVE
		do
		{
			struct RectMenu *next = menu->ptrNextBox_InHierarchy;
#endif

			// Close menu
			menu->state |= 8;
			menu->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);

			// Reset ptrNext and ptrPrev
			menu->ptrNextBox_InHierarchy = 0;
			menu->ptrPrevBox_InHierarchy = 0;

#ifdef CTR_NATIVE
			menu = next;
		} while (menu != 0);
#endif
	}

	// unused
	sdata->framesRemainingInMenu = 0xF;
}
