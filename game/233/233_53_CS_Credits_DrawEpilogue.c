#include <common.h>

extern struct Ovr233_Credits_BSS *creditsBSS;

void CS_Credits_DrawEpilogue(struct CreditsObj *co)
{
	if (co->epilogue_topString == 0)
		return;

	co->epilogueCount200--;

	if (co->epilogueCount200 <= 0)
	{
		co->epilogueCount200 = 200;
		co->epilogue_topString = co->epilogue_nextString;
		co->epilogue_nextString = CS_Credits_GetNextString(co->epilogue_nextString);
	}

	if (co->epilogue_topString == 0)
		return;

	short timeRemaining = co->epilogueCount200;
	short fadeAmount = 20;

	if (timeRemaining < 0xb5)
	{
		if (timeRemaining <= 0x13)
		{
			fadeAmount = timeRemaining;
		}
	}
	else
	{
		fadeAmount = 200 - timeRemaining;
	}

	short colorSlot = 4;

	if (fadeAmount > 0)
	{
		colorSlot = 0x1f;

		int fade8 = (fadeAmount << 8) / 20;
		char *dst = (char *)&data.colors[31];
		char *src = (char *)data.ptrColor[4];

		for (int i = 0; i < 4; i++)
		{
			int stride = i * 4;
			dst[stride + 0] = (char)((unsigned char)src[stride + 0] * fade8 >> 8);
			dst[stride + 1] = (char)((unsigned char)src[stride + 1] * fade8 >> 8);
			dst[stride + 2] = (char)((unsigned char)src[stride + 2] * fade8 >> 8);
		}
	}
	else
	{
		colorSlot = -1;
	}

	if ((colorSlot >= 0) && (creditsBSS->boolAllBlue != 0))
	{
		short strLen = -1;

		if (co->epilogue_nextString != 0)
		{
			strLen = (short)(co->epilogue_nextString - co->epilogue_topString) - 1;
		}

		DECOMP_DecalFont_DrawMultiLineStrlen(co->epilogue_topString, strLen, 0x100, 0xaf, 0x1cc, 2, colorSlot | 0x8000);
	}
}
