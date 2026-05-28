#include <common.h>

static void SelectProfile_DrawAdvProfile_UpdateIcon(struct SelectProfileLoadSaveObj *obj, int index, int posX, int posY)
{
	struct Instance *inst = obj->icons[index].inst;

	inst->matrix.t[0] = SelectProfile_UI_ConvertX(posX, 0x100);
	inst->matrix.t[1] = SelectProfile_UI_ConvertY(posY, 0x100);
	inst->matrix.t[2] = 0x100;
	inst->flags &= ~HIDE_MODEL;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80047ff8-0x800485a8.
void SelectProfile_DrawAdvProfile(struct AdvProgress *adv, int posX, int posY, s16 isHighlighted, s16 slotIndex, u16 menuFlag)
{
	struct GameTracker *gGT = sdata->gGT;
	RECT profileRect;

	int iconColor;
	int numberColor;
	int emptyColor;
	int nameColor;
	int percentColor;

	if ((menuFlag & 0x10) != 0)
	{
		iconColor = sdata->greenColor;
		numberColor = 0x1d;
		emptyColor = 0x1e;
		nameColor = 0x1d;
		percentColor = 0x1d;
	}
	else
	{
		iconColor = sdata->greyColor;
		numberColor = 0;
		emptyColor = 3;
		nameColor = 1;
		percentColor = 4;
	}

	slotIndex *= 3;
	GAMEPROG_AdvPercent(adv);

	if (adv->characterID < 0)
	{
		DecalFont_DrawLine(sdata->lngStrings[0xb5], posX + 0x6c, posY + 0x17, FONT_BIG, emptyColor | 0xffff8000);
	}
	else
	{
		int profileTextColor = numberColor | 0x4000;
		int characterID = adv->characterID;
		int iconID = data.MetaDataCharacters[characterID].iconID;
		struct SelectProfileLoadSaveObj *obj = (struct SelectProfileLoadSaveObj *)sdata->ptrLoadSaveObj;

		RECTMENU_DrawPolyGT4(gGT->ptrIcons[iconID], posX + 10, posY + 6, &gGT->backBuffer->primMem, gGT->backBuffer->otMem.startPlusFour, iconColor, iconColor,
		                     iconColor, iconColor, 1, 0x1000);

		DecalFont_DrawLine(adv->name, posX + 0x6c, posY + 0x29, FONT_BIG, nameColor | 0xffff8000);

		SelectProfile_PrintInteger(gGT->currAdvProfile.completionPercent, posX + 0x6a, posY + 0x17, 0, profileTextColor);
		SelectProfile_PrintInteger(gGT->currAdvProfile.numTrophies, posX + 0x6a, posY + 5, 0, profileTextColor);
		SelectProfile_PrintInteger(gGT->currAdvProfile.numKeys, posX + 0xb5, posY + 5, 0, profileTextColor);
		SelectProfile_PrintInteger(gGT->currAdvProfile.numRelics, posX + 0xb5, posY + 0x17, 0, profileTextColor);

		DecalFont_DrawLine((char *)&sdata->s_percent_sign, posX + 0x70, posY + 0x17, FONT_BIG, percentColor);

		SelectProfile_DrawAdvProfile_UpdateIcon(obj, slotIndex, posX + 0xc3, posY + 0x1f);
		SelectProfile_DrawAdvProfile_UpdateIcon(obj, slotIndex + 1, posX + 0x78, posY + 0xd);
		SelectProfile_DrawAdvProfile_UpdateIcon(obj, slotIndex + 2, posX + 0xc3, posY + 0xd);
	}

	profileRect.x = posX;
	profileRect.y = posY;
	profileRect.w = 0xdc;
	profileRect.h = 0x3d;

	if (isHighlighted != 0)
	{
		RECT highlightRect;
		Color *highlightColor = ((menuFlag & 0x10) != 0) ? &sdata->menuRowHighlight_Green : &sdata->menuRowHighlight_Normal;

		highlightRect.x = posX + 6;
		highlightRect.y = posY + 4;
		highlightRect.w = 0xd0;
		highlightRect.h = 0x35;

		CTR_Box_DrawClearBox(&highlightRect, highlightColor, 1, &gGT->backBuffer->otMem.startPlusFour[3]);
	}

	RECTMENU_DrawInnerRect(&profileRect, (s16)menuFlag, &gGT->backBuffer->otMem.startPlusFour[3]);
}
