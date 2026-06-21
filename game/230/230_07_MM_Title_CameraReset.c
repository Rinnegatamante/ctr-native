#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac92c-0x800ac94c.
void MM_Title_CameraReset(void)
{
	struct Title *title = D230.titleObj;

	if (title == NULL)
		return;

	title->cameraPosOffset.x = 2000;
}
