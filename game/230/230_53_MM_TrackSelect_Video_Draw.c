#include <common.h>

#ifdef CTR_NATIVE
#include <platform/native_str.h>

static void MM_TrackSelect_Video_DrawNativePreview(RECT *r, int srcX, int srcY)
{
	struct GameTracker *gGT = sdata->gGT;
	u32 *prim = (u32 *)gGT->backBuffer->primMem.cursor;
	u_long *ot = gGT->pushBuffer_UI.ptrOT;
	u32 oldTag = (u32)*ot;
	u32 *nextPrim;
	s16 tile[16] = {
	    (s16)srcX, (s16)srcY, 0xaa, 0x47, (s16)(r->x + 3), (s16)(r->y + 2), 0xaa, 0x47,
	};

	// NOTE(aalhendi): Retail copies decoded MDEC output with MoveImage. Native
	// presents menus from queued primitives, so draw the same VRAM rectangle as
	// a 16-bit textured quad instead of relying on a CPU-side VRAM copy.
	*ot = (u_long)CtrGpu_PrimToOTLink24(prim);
	nextPrim = DISPLAY_Blur_SubFunc(prim, tile);
	nextPrim[-10] = oldTag | 0x09000000;
	gGT->backBuffer->primMem.cursor = nextPrim;
}
#endif

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800afaf0-0x800aff58 PSX path.
void MM_TrackSelect_Video_Draw(RECT *r, struct MainMenu_LevelRow *selectMenu, int trackIndex, int param_4, u16 param_5)
{
	u8 u0;
	u8 v0;
	u16 tpage;
	int srcX;
	int srcY;
	struct GameTracker *gGT = sdata->gGT;
	struct BigHeader *bh = sdata->ptrBigfileCdPos_2;
	struct BigEntry *entry = BIG_GETENTRY(bh);
	int videoID;

	selectMenu = &selectMenu[trackIndex];
	videoID = selectMenu->videoID;

	if ((entry[videoID].size == 0) ||

	    // Video off-screen
	    (r->x < 0) || (r->y < 0) || ((r->x + r->w) > 0x200) || ((r->y + r->h) > 0xd8))
	{
		// draw icon
		D230.trackSel_videoStateCurr = 1;
	}
#ifdef CTR_NATIVE
	else
	{
		if ((D230.trackSel_videoStateCurr == 2) && (D230.trackSel_videoStatePrev == 1))
		{
			if (NativeSTR_StartTrackPreview(videoID, selectMenu->videoLength) != 0)
				D230.trackSel_video_boolAllocated = D230.trackSel_videoStatePrev;
		}

		if (((D230.trackSel_videoStatePrev == 3) || (D230.trackSel_videoStateCurr == 3)) || (D230.trackSel_videoStateCurr == 2))
		{
			int uploaded;

			tpage = gGT->ptrIcons[0x3f]->texLayout.tpage;
			u0 = gGT->ptrIcons[0x3f]->texLayout.u0;
			v0 = gGT->ptrIcons[0x3f]->texLayout.v0;
			srcX = (u16)u0 + (tpage & 0xf) * 0x40;
			srcY = (u16)v0 + (tpage & 0x10) * 0x10 + (s16)(((u32)tpage & 0x800) >> 2);
			uploaded = NativeSTR_UploadNextFrame(srcX, srcY);

			if ((uploaded == 1) && (D230.trackSel_videoStateCurr == 2))
				D230.trackSel_videoStateCurr = 3;

			if (D230.trackSel_videoStateCurr == 3)
				MM_TrackSelect_Video_DrawNativePreview(r, srcX + 3, srcY + 2);
		}
	}
#else
	else
	{
		if ((D230.trackSel_videoStateCurr == 2) && (D230.trackSel_videoStatePrev == 1))
		{
			// If you have not allocated memory for video yet
			if (D230.trackSel_video_boolAllocated == 0)
			{
				// Allocate memory for video in Track Selection
				MM_Video_AllocMem(0xb0, 0x4b, 4, 0, 0);

				// You have now allocated the memory
				D230.trackSel_video_boolAllocated = D230.trackSel_videoStatePrev;
			}

			// CD position of video, and numFrames
			MM_Video_StartStream(bh->cdpos + entry[videoID].offset, selectMenu->videoLength);
		}

		if (((D230.trackSel_videoStatePrev == 3) || (D230.trackSel_videoStateCurr == 3)) || (D230.trackSel_videoStateCurr == 2))
		{
			tpage = gGT->ptrIcons[0x3f]->texLayout.tpage;
			u0 = gGT->ptrIcons[0x3f]->texLayout.u0;
			v0 = gGT->ptrIcons[0x3f]->texLayout.v0;
			srcX = (u16)u0 + (tpage & 0xf) * 0x40;
			srcY = (u16)v0 + (tpage & 0x10) * 0x10 + (s16)(((u32)tpage & 0x800) >> 2);

			// Decode into the icon's VRAM page; the copied rectangle starts inside it.
			int ret = MM_Video_DecodeFrame(srcX, srcY);

			if ((ret == 1) && (D230.trackSel_videoStateCurr == 2))
			{
				D230.trackSel_videoStateCurr = 3;
			}
			if (D230.trackSel_videoStatePrev == 3)
			{
				// RECT position (x,y)
				sdata->videoSTR_src_vramRect.x = srcX + 3;
				sdata->videoSTR_src_vramRect.y = srcY + 2;

				// RECT size (w,h)
				sdata->videoSTR_src_vramRect.w = 0xaa;
				sdata->videoSTR_src_vramRect.h = 0x47;

				// VRAM destination (x,y) on swapchain image
				sdata->videoSTR_dst_vramX = gGT->db[gGT->swapchainIndex].dispEnv.disp.x + (r->x + 3);
				sdata->videoSTR_dst_vramY = gGT->db[gGT->swapchainIndex].dispEnv.disp.y + (r->y + 2);

				// enable video copy, give src and dst
				MainFrame_InitVideoSTR(1, &sdata->videoSTR_src_vramRect, sdata->videoSTR_dst_vramX, sdata->videoSTR_dst_vramY);
			}
		}
	}
#endif

	// if not playing video, draw icon
	if (D230.trackSel_videoStateCurr != 3)
	{
		// Draw Video icon
		RECTMENU_DrawPolyGT4(gGT->ptrIcons[selectMenu->videoThumbnail], (r->x + 3), (r->y + 2), &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT,
		                     D230.videoCol, D230.videoCol, D230.videoCol, D230.videoCol, 0, FP(1.0));
	}

#ifndef CTR_NATIVE
	if (D230.trackSel_videoStatePrev == 1)
	{
		// disable video copy
		MainFrame_InitVideoSTR(0, 0, 0, 0);
	}

	// First frame to start video
	if ((param_4 == 1) && (D230.trackSel_video_boolAllocated == 1))
	{
		D230.trackSel_videoStateCurr = 1;
	}

	// First frame to stop video
	if ((D230.trackSel_videoStateCurr == 1) && (D230.trackSel_videoStatePrev != 1))
	{
		MM_Video_StopStream();
	}

	// First frame to start video,
	// but this time after stopping video safely
	if ((param_4 == 1) && (D230.trackSel_video_boolAllocated == 1))
	{
		MM_Video_ClearMem();

		D230.trackSel_video_boolAllocated = 0;
	}
#else
	if (D230.trackSel_videoStatePrev == 1)
	{
		MainFrame_InitVideoSTR(0, 0, 0, 0);
	}

	if ((param_4 == 1) && (D230.trackSel_video_boolAllocated == 1))
	{
		D230.trackSel_videoStateCurr = 1;
	}

	if ((D230.trackSel_videoStateCurr == 1) && (D230.trackSel_videoStatePrev != 1))
	{
		NativeSTR_Stop();
	}

	if ((param_4 == 1) && (D230.trackSel_video_boolAllocated == 1))
	{
		NativeSTR_Stop();
		D230.trackSel_video_boolAllocated = 0;
	}
#endif

	D230.trackSel_videoStatePrev = D230.trackSel_videoStateCurr;

	// Draw 2D Menu rectangle background
	RECTMENU_DrawInnerRect(r, (s16)(param_5 | 1), gGT->backBuffer->otMem.uiOT);
}
