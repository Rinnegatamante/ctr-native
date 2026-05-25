#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8001c420-0x8001c470.
u32 CDSYS_GetFilePosInt(char *fileString, int *filePos)
{
	CdlFILE cdlFile;

	if (CdSearchFile(&cdlFile, fileString) != 0)
	{
		*filePos = CdPosToInt(&cdlFile.pos);
		return 1;
	}

	return 0;
}
