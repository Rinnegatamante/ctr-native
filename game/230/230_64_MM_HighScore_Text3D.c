#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800b2f0c-0x800b2fbc.
void MM_HighScore_Text3D(char *string, int posX, int posY, s16 font, u32 flags)
{
	// draw a string
	DecalFont_DrawLine(string, posX, posY, font, flags);

	// draw the same string in a different place
	DecalFont_DrawLine(string, (posX + 3), (posY + 1), font, (flags & (JUSTIFY_CENTER | JUSTIFY_RIGHT)) | BLACK);
}
