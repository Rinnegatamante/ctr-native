#include <common.h>

void DECOMP_MM_HighScore_Draw(u16 trackIndex, u32 rowIndex, u32 posX, u32 posY)
{
	char i;
	s16 numColor;
	s16 sVar1;
	s16 lineWidth;
	int *colorPtr;
	s16 levelID;
	struct HighScoreEntry *entry;
	u32 uVar9;
	u32 uVar10;
	RECT videoBox;
	s16 offsetX;
	s16 offsetY;

	struct GameTracker *gGT = sdata->gGT;

	offsetX = (s16)posX;
	uVar10 = posX & 0xffff;
	offsetY = (s16)posY;
	uVar9 = posY & 0xffff;

	levelID = D230.arcadeTracks[trackIndex].levID;

	lineWidth = DECOMP_DecalFont_GetLineWidth(sdata->lngStrings[data.metaDataLEV[levelID].name_LNG], 1);
	lineWidth = lineWidth >> 1;

	// get color data
	numColor = ((sdata->frameCounter & 4) == 0) ? RED : ORANGE;
	colorPtr = data.ptrColor[numColor];

	struct Icon **iconPtrArray = ICONGROUP_GETICONS(gGT->iconGroup[4]);

	// Draw arrow pointing Left
	DecalHUD_Arrow2D(iconPtrArray[0x38], D230.transitionMeta_HighScores[0].currX + (offsetX - lineWidth) + 0xec,
	                 D230.transitionMeta_HighScores[0].currY + offsetY + 0x15, &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT, colorPtr[0], colorPtr[1],
	                 colorPtr[2], colorPtr[3], 0, 0x1000, 0x800);

	// Draw arrow pointing Right
	DecalHUD_Arrow2D(iconPtrArray[0x38], D230.transitionMeta_HighScores[0].currX + (lineWidth + offsetX) + 0x112,
	                 D230.transitionMeta_HighScores[0].currY + offsetY + 0x15, &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT, colorPtr[0], colorPtr[1],
	                 colorPtr[2], colorPtr[3], 0, 0x1000, 0);

	// draw track name
	DECOMP_DecalFont_DrawLine(sdata->lngStrings[data.metaDataLEV[levelID].name_LNG], D230.transitionMeta_HighScores[0].currX + (s16)(posX + 0x100),
	                          D230.transitionMeta_HighScores[0].currY + (s16)(posY + 0xe), FONT_BIG, JUSTIFY_CENTER);

	u32 iconColor = D230.highscore_iconColor;

	// "BEST TRACK TIMES"
	DECOMP_MM_HighScore_Text3D(sdata->lngStrings[0xb3], D230.transitionMeta_HighScores[1].currX + (uVar10 + 0x20),
	                           D230.transitionMeta_HighScores[1].currY + (uVar9 + 0x2b), FONT_SMALL, 0);

	// first entry: Time Trial or Relic
	entry = &sdata->gameProgress.highScoreTracks[levelID].scoreEntry[rowIndex * 6];

	// if Time Trial
	// with ghost stars, and Best Lap
	if ((rowIndex & 0xffff) == 0)
	{
		// draw ghost stars
		for (i = 0; i < 2; i++)
		{
			if (((sdata->gameProgress.highScoreTracks[levelID].timeTrialFlags >> D230.highscore_ghostBeatFlags[i]) & 1) != 0)
			{
				colorPtr = data.ptrColor[D230.colorIndexArray[i]];

				struct Icon **ptrIconArray;
				ptrIconArray = ICONGROUP_GETICONS(gGT->iconGroup[5]);

				DecalHUD_DrawPolyGT4(ptrIconArray[0x37], D230.transitionMeta_HighScores[0].currX + offsetX + (i * 0x10) + 0xf0,
				                     D230.transitionMeta_HighScores[0].currY + offsetY + 4,
				                     // pointer to PrimMem struct
				                     &gGT->backBuffer->primMem,
				                     // pointer to OT mem
				                     gGT->pushBuffer_UI.ptrOT, colorPtr[0], colorPtr[1], colorPtr[2], colorPtr[3], 0, 0x1000);
			}
		}

		// "BEST LAP TIME:"
		DECOMP_MM_HighScore_Text3D(sdata->lngStrings[0xb4], D230.transitionMeta_HighScores[7].currX + (uVar10 + 0x124),
		                           D230.transitionMeta_HighScores[7].currY + (uVar9 + 0x2b), FONT_SMALL, 0);

		// Character Name
		DECOMP_MM_HighScore_Text3D(entry[0].name, D230.transitionMeta_HighScores[7].currX + (uVar10 + 0x160),
		                           D230.transitionMeta_HighScores[7].currY + (uVar9 + 0x39), FONT_BIG, entry[0].characterID + 5);

		// Draw time string
		DECOMP_MM_HighScore_Text3D(DECOMP_RECTMENU_DrawTime(entry[0].time), D230.transitionMeta_HighScores[7].currX + (uVar10 + 0x160),
		                           D230.transitionMeta_HighScores[7].currY + (uVar9 + 0x4a), FONT_SMALL, 0);

		// Character Icon
		DECOMP_RECTMENU_DrawPolyGT4(gGT->ptrIcons[data.MetaDataCharacters[entry[0].characterID].iconID],
		                            D230.transitionMeta_HighScores[7].currX + (offsetX + 0x124), D230.transitionMeta_HighScores[7].currY + (offsetY + 0x38),
		                            &gGT->backBuffer->primMem, (gGT->pushBuffer_UI).ptrOT, iconColor, iconColor, iconColor, iconColor, 1, 0x1000);
	}

	// Draw five "best track times"
	// Icon, Name, and Time
	for (i = 0; i < 5; i++)
	{
		char j = i + 2;

		// Character Icon
		DECOMP_RECTMENU_DrawPolyGT4(gGT->ptrIcons[data.MetaDataCharacters[entry[i + 1].characterID].iconID],
		                            D230.transitionMeta_HighScores[j].currX + offsetX + 0x20,
		                            D230.transitionMeta_HighScores[j].currY + offsetY + (i * 0x1f) + 0x39, &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT,
		                            iconColor, iconColor, iconColor, iconColor, 1, 0x1000);

		// draw the name string
		DECOMP_MM_HighScore_Text3D(entry[i + 1].name, D230.transitionMeta_HighScores[j].currX + uVar10 + 0x5c,
		                           D230.transitionMeta_HighScores[j].currY + uVar9 + (i * 0x1f) + 0x39, FONT_BIG, entry[i + 1].characterID + 5);

		// draw the Time string
		DECOMP_MM_HighScore_Text3D(DECOMP_RECTMENU_DrawTime(entry[i + 1].time), D230.transitionMeta_HighScores[j].currX + uVar10 + 0x5c,
		                           D230.transitionMeta_HighScores[j].currY + uVar9 + (i * 0x1f) + 0x4a, FONT_SMALL, 0);
	}

	videoBox.w = 0xb0;
	videoBox.h = 0x4b;
	videoBox.x = D230.transitionMeta_HighScores[9].currX + offsetX + 0x124;
	videoBox.y = D230.transitionMeta_HighScores[9].currY + offsetY + 0x5a;

	DECOMP_MM_TrackSelect_Video_Draw(&videoBox, &D230.arcadeTracks[0], trackIndex, (D230.highScore_transitionState == EXITING_MENU), 0);
}
