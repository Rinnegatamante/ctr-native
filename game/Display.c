#include <common.h>


static u32 DISPLAY_Blur_Ptr24(const void *ptr)
{
	return CtrGpu_PrimToOTLink24(ptr);
}

static u32 DISPLAY_Blur_PackS16Pair(int x, int y)
{
	return ((u32)(u16)x) | ((u32)(u16)y << 16);
}

static void DISPLAY_Blur_WriteLo16(u32 *word, u16 value)
{
	*(u16 *)word = value;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80023a40-0x80023d4c
u32 *DISPLAY_Blur_SubFunc(u32 *prim, s16 *tile)
{
	int srcX = tile[0];
	int srcY = tile[1];
	int srcW = tile[2];
	int srcH = tile[3];

	if (((srcX + srcW) & -0x100) != (srcX & -0x100))
	{
		tile[10] = (s16)((srcX & -0x100) - (srcX - 0xff));
		tile[14] = (s16)((tile[10] * tile[6]) / srcW);

		tile[8] = (s16)srcX;
		tile[9] = (s16)srcY;
		tile[11] = (s16)srcH;
		tile[15] = tile[7];
		tile[12] = tile[4];
		tile[13] = tile[5];

		if (tile[6] != 0)
			prim = DISPLAY_Blur_SubFunc(prim, tile + 8);

		tile[8] = (s16)(tile[10] + tile[8] + 1);
		tile[10] = (s16)(tile[2] - tile[10] - 1);
		tile[12] = (s16)(tile[12] + tile[14]);
		tile[14] = (s16)(tile[6] - tile[14]);

		if (tile[6] != 0)
			prim = DISPLAY_Blur_SubFunc(prim, tile + 8);

		return prim;
	}

	if (((srcY + srcH) & -0x100) != (srcY & -0x100))
	{
		tile[11] = (s16)((srcY & -0x100) - (srcY - 0xff));
		tile[15] = (s16)((tile[11] * tile[7]) / srcH);

		tile[8] = (s16)srcX;
		tile[9] = (s16)srcY;
		tile[10] = (s16)srcW;
		tile[14] = tile[6];
		tile[12] = tile[4];
		tile[13] = tile[5];

		if (tile[7] != 0)
			prim = DISPLAY_Blur_SubFunc(prim, tile + 8);

		tile[9] = (s16)(tile[11] + tile[9] + 1);
		tile[11] = (s16)(tile[3] - tile[11] - 1);
		tile[13] = (s16)(tile[13] + tile[15]);
		tile[15] = (s16)(tile[7] - tile[15]);

		if (tile[7] != 0)
			prim = DISPLAY_Blur_SubFunc(prim, tile + 8);

		return prim;
	}

	u32 u0 = (u32)srcX & 0x3f;
	u32 v0 = ((u32)srcY & 0xff) << 8;
	u32 u1 = (u32)(srcW + (int)u0);
	u32 v1 = v0 + ((u32)srcH << 8);
	int dstX = tile[4];
	int dstY = tile[5];
	int dstW = tile[6];
	int dstH = tile[7];
	u32 tpage = (((u32)srcY & 0x100) >> 4) | (((u32)srcX & 0x3ff) >> 6) | 0x100 | (((u32)srcY & 0x200) << 2);

	prim[3] = u0 | v0;
	prim[2] = DISPLAY_Blur_PackS16Pair(dstX, dstY);
	prim[4] = DISPLAY_Blur_PackS16Pair(dstX + dstW, dstY);
	prim[6] = DISPLAY_Blur_PackS16Pair(dstX, dstY + dstH);
	prim[8] = DISPLAY_Blur_PackS16Pair(dstX + dstW, dstY + dstH);
	DISPLAY_Blur_WriteLo16(&prim[7], (u16)(u0 | v1));
	DISPLAY_Blur_WriteLo16(&prim[9], (u16)(u1 | v1));
	*(u8 *)((u8 *)prim + 7) = 0x2f;
	prim[0] = DISPLAY_Blur_Ptr24(prim + 10) | 0x09000000;
	prim[5] = u1 | v0 | (tpage << 16);

	return prim + 10;
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80023d4c-0x80023ffc
void DISPLAY_Blur_Main(struct PushBuffer *pb, int strength)
{
	struct GameTracker *gGT = sdata->gGT;
	struct DB *backBuffer = gGT->backBuffer;
	u32 *prim = backBuffer->primMem.cursor;
	u32 *nextPrim;
	s8 cameraID;
	u_long *ot;

	cameraID = *(s8 *)&pb->cameraID;

	if (strength < 1 || (((gGT->db[1 - gGT->swapchainIndex].blurCameraMask >> (cameraID & 0x1f)) & 1) == 0))
	{
		int x = pb->rect.x;
		int y = pb->rect.y;
		int w = pb->rect.w;
		int h = pb->rect.h;

		prim[8] = 0xe1000a00;
		prim[1] = 0xe1000a20;
		prim[2] = 0xe6000001;
		prim[9] = 0xe6000000;
		prim[4] = DISPLAY_Blur_PackS16Pair(x, y);
		prim[5] = DISPLAY_Blur_PackS16Pair(x + w, y);
		prim[6] = DISPLAY_Blur_PackS16Pair(x, y + h);
		prim[7] = DISPLAY_Blur_PackS16Pair(x + w, y + h);
		prim[3] = (strength < 0) ? 0x2affffff : 0x2a000000;

		ot = gGT->otSwapchainDB[gGT->swapchainIndex];
		prim[0] = (u32)*ot | 0x09000000;
		*ot = (u_long)DISPLAY_Blur_Ptr24(prim);
		nextPrim = prim + 10;
	}
	else
	{
		int wave = gGT->timer + cameraID;
		s16 *scratch = CTR_SCRATCHPAD_PTR(s16, 0);
		u32 oldTag;
		int blur;
		int insetX;
		int insetY;

		if ((cameraID & 1) != 0)
			wave = -wave;

		blur = MATH_Sin(wave * 100);
		if (blur < 0)
			blur = -blur;

		blur = (blur >> 2) + 0x400;
		if (strength < 0x1000)
			blur = (blur * strength) >> 12;

		ot = gGT->otSwapchainDB[gGT->swapchainIndex];
		oldTag = *ot;
		*ot = (u_long)DISPLAY_Blur_Ptr24(prim);

		scratch[4] = pb->rect.x;
		scratch[5] = pb->rect.y;
		scratch[6] = pb->rect.w;
		scratch[7] = pb->rect.h;

		insetX = ((blur * 9) >> 12) + 2;
		scratch[0] = backBuffer->dispEnv.disp.x + pb->rect.x + insetX;
		scratch[2] = pb->rect.w - (insetX * 2);

		insetY = ((blur * 6) >> 12) + 2;
		scratch[1] = backBuffer->dispEnv.disp.y + pb->rect.y + insetY;
		scratch[3] = pb->rect.h - (insetY * 2);

		nextPrim = DISPLAY_Blur_SubFunc(prim, scratch);
		nextPrim[-10] = oldTag | 0x09000000;
	}

	backBuffer->primMem.cursor = nextPrim;
	backBuffer->blurCameraMask |= (u8)(1 << (cameraID & 0x1f));
}


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80023ffc-0x8002406c.
void DISPLAY_Swap(void)
{
	struct GameTracker *gGT;
	struct DB *db;

	gGT = sdata->gGT;

	// get pointer to the "new" frontBufferDB, which is current backBuffer
	db = &gGT->db[gGT->swapchainIndex];

	// flip swapchain index (0->1) (1->0)
	gGT->swapchainIndex = 1 - gGT->swapchainIndex;

	// Set value of frontBuffer DB
	gGT->frontBuffer = db;

	// frontBuffer->dispEnv
	PutDispEnv(&db->dispEnv);

	// frontBuffer
	PutDrawEnv(&db->drawEnv);
	return;
}
