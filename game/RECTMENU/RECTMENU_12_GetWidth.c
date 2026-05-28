#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80045c50-0x80045db0.
void RECTMENU_GetWidth(struct RectMenu *m, s16 *width, int boolCheckSubmenu)
{
	int fontType;
	struct MenuRow *row;
	int lineWidth;

	fontType = FONT_BIG;

	// if menu should have tiny text
	if ((m->state & 0x80) != 0)
	{
		fontType = FONT_SMALL;
	}

	// handle rows
	for (row = m->rows; row->stringIndex != -1; row++)
	{
		// width of string in each row
		lineWidth = DecalFont_GetLineWidth(sdata->lngStrings[row->stringIndex & 0x7fff], fontType);

		// set new width if new max is found
		if (*width < (lineWidth + 1))
		{
			*width = lineWidth + 1;
		}
	}

	// handle menu title
	if (m->stringIndexTitle >= 0)
	{
		// if force title to be big
		if ((m->state & 0x4000) != 0)
		{
			// override
			fontType = FONT_BIG;
		}

		// width of string in each row
		lineWidth = DecalFont_GetLineWidth(sdata->lngStrings[m->stringIndexTitle & 0x7fff], fontType);

		// set new width if new max is found
		if (*width < (lineWidth + 1))
		{
			*width = lineWidth + 1;
		}
	}

	// if submenu needs to be drawn
	if ((m->state & 0x10) != 0)
	{
		if ((boolCheckSubmenu & 0xffff) != 0)
		{
			// recursively check height for more submenus
			RECTMENU_GetWidth(m->ptrNextBox_InHierarchy, width, 1);
		}
	}
}
