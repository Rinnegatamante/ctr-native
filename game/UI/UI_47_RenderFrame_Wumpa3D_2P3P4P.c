#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005465c-0x80054a08.
void UI_RenderFrame_Wumpa3D_2P3P4P(struct GameTracker *gGT)
{
	RECT viewport2P;
	RECT viewport3P4P;
	RECT *viewport;
	struct PushBuffer *wumpaPushBuffer;
	u32 packedRect;

	// NOTE(aalhendi): Retail copies packed RECT halfwords; native unpacks them
	// explicitly to avoid strict-aliasing UB from writing RECT fields as u32.
	packedRect = sdata->multiplayerWumpaHudData[0];
	viewport2P.x = (s16)packedRect;
	viewport2P.y = (s16)(packedRect >> 16);
	packedRect = sdata->multiplayerWumpaHudData[1];
	viewport2P.w = (s16)packedRect;
	viewport2P.h = (s16)(packedRect >> 16);
	packedRect = sdata->multiplayerWumpaHudData[2];
	viewport3P4P.x = (s16)packedRect;
	viewport3P4P.y = (s16)(packedRect >> 16);
	packedRect = sdata->multiplayerWumpaHudData[3];
	viewport3P4P.w = (s16)packedRect;
	viewport3P4P.h = (s16)(packedRect >> 16);

	viewport = &viewport2P;
	if (gGT->numPlyrCurrGame >= 3)
		viewport = &viewport3P4P;

	// NOTE(aalhendi): Retail reads the gp slot populated by UI_INSTANCE_InitAll
	// with ptrPushBufferUI, not the adjacent ptrFruitDisp instance slot.
	wumpaPushBuffer = (struct PushBuffer *)(uintptr_t)sdata->ptrPushBufferUI;

	if (wumpaPushBuffer != NULL)
	{
		PushBuffer_SetDrawEnv_DecalMP(wumpaPushBuffer->renderBucketOTRangeEnd, gGT->backBuffer, viewport, viewport->x + (viewport->w >> 1) - 0x100,
		                              viewport->y + (viewport->h >> 1) - 0x6c, 0, 0, 0, 0, 1);

		u32 *textureStart = wumpaPushBuffer->ptrOT;
		u32 *textureEnd = wumpaPushBuffer->renderBucketOTRangeEnd;

		if (textureStart != NULL && textureEnd != NULL)
		{
			CTR_CycleTex_2p3p4pWumpaHUD((u32 *)&gGT->pushBuffer[0].ptrOT[0x3ff], textureStart, (int)(textureEnd - textureStart) + 1);
		}
	}

	if (gGT->numPlyrCurrGame < 2)
		return;

	struct UiElement2D *hud = data.hudStructPtr[gGT->numPlyrCurrGame - 1];

	for (int playerIndex = 0; playerIndex < gGT->numPlyrCurrGame; playerIndex++, hud += 0x14)
	{
		struct Driver *driver = gGT->drivers[playerIndex];

		if ((driver->actionsFlagSet & ACTION_RACE_FINISHED) != 0)
			continue;

		if ((gGT->gameMode1 & END_OF_RACE) != 0)
			continue;

		s16 posX = hud[3].x + wumpaPushBuffer->rect.x - (viewport->w >> 1);
		s16 posY = hud[3].y + wumpaPushBuffer->rect.y - (viewport->h >> 1);

		u32 *prim = (u32 *)gGT->backBuffer->primMem.curr;

		*(u8 *)((u8 *)prim + 3) = 9;
		*(u8 *)((u8 *)prim + 7) = 0x2c;
		*(u8 *)((u8 *)prim + 6) = 0x80;
		*(u8 *)((u8 *)prim + 5) = 0x80;
		*(u8 *)((u8 *)prim + 4) = 0x80;

		*(s16 *)((u8 *)prim + 0x08) = posX;
		*(s16 *)((u8 *)prim + 0x0a) = posY;
		*(s16 *)((u8 *)prim + 0x12) = posY;
		*(s16 *)((u8 *)prim + 0x18) = posX;
		*(s16 *)((u8 *)prim + 0x10) = posX + viewport->w;
		*(s16 *)((u8 *)prim + 0x1a) = posY + viewport->h;
		*(s16 *)((u8 *)prim + 0x20) = posX + viewport->w;
		*(s16 *)((u8 *)prim + 0x22) = posY + viewport->h;

		*(u8 *)((u8 *)prim + 0x0c) = viewport->x & 0x3f;
		*(u8 *)((u8 *)prim + 0x0d) = viewport->y;
		*(u8 *)((u8 *)prim + 0x14) = *(u8 *)((u8 *)prim + 0x0c) + (u8)viewport->w;
		*(u8 *)((u8 *)prim + 0x15) = *(u8 *)((u8 *)prim + 0x0d);
		*(u8 *)((u8 *)prim + 0x1c) = *(u8 *)((u8 *)prim + 0x0c);
		*(u8 *)((u8 *)prim + 0x1d) = *(u8 *)((u8 *)prim + 0x0d) + (u8)viewport->h;
		*(u8 *)((u8 *)prim + 0x24) = *(u8 *)((u8 *)prim + 0x0c) + (u8)viewport->w;
		*(u8 *)((u8 *)prim + 0x25) = *(u8 *)((u8 *)prim + 0x0d) + (u8)viewport->h;

		u16 tpage = (u16)(((viewport->y & 0x100) >> 4) | ((viewport->x & 0x3ff) >> 6) | 0x100 | ((viewport->y & 0x200) << 2));
		*(u16 *)((u8 *)prim + 0x16) = tpage;

		if (driver->numWumpas >= 10)
		{
			u8 shineColor = sdata->wumpaShineColor1[0][0];
			*(u8 *)((u8 *)prim + 6) = shineColor;
			*(u8 *)((u8 *)prim + 5) = shineColor;
			*(u8 *)((u8 *)prim + 4) = shineColor;
		}

		gGT->backBuffer->primMem.curr = prim + 10;
		AddPrim(gGT->pushBuffer_UI.ptrOT, prim);
	}
}
