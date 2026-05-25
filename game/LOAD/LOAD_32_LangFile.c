#include <common.h>

struct LngFile
{
	int numStrings;
	int offsetToPtrArr;
	char strings[1];
};

// param_1 - Pointer to "cd position of bigfile"
// param_2 - language index - 0 ja, 1 en, 2 en2, 3 fr, 4 de, 5 it, 6 es, 7 ne
// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80032b50-0x80032c24
void DECOMP_LOAD_LangFile(int bigfilePtr, int lang)
{
	struct LngFile *lngFile;
	int size;

	int i;
	int numStrings;
	char **strArray;

#if BUILD == EurRetail
	// This is to turn the screen black for a bit (optional)
	CTR_ErrorScreen(0, 0, 0);
	VSync(0);
#endif

	if (sdata->lngFile == 0)
	{
		sdata->lngFile = MEMPACK_AllocMem(sdata->langBufferSize /* "lang buffer" */);
	}

	lngFile = sdata->lngFile;

	lngFile = DECOMP_LOAD_ReadFile(bigfilePtr, LT_SETADDR | LT_SYNC, BI_LANGUAGEFILE + lang, (void *)lngFile);
	if (lngFile == NULL)
		return;

	numStrings = lngFile->numStrings;
	strArray = (char **)((u32)lngFile + lngFile->offsetToPtrArr);

	sdata->numLngStrings = numStrings;
	sdata->lngStrings = strArray;

	for (i = 0; i < numStrings; i++)
	{
		strArray[i] = (char *)((u32)strArray[i] + (u32)lngFile);
	}
#if BUILD == EurRetail
	// set voicelines to new lang
	CDSYS_SetXAToLang(lang);
#endif
}
