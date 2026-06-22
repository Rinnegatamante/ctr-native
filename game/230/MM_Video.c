#include <common.h>

#ifndef CTR_NATIVE

void MM_Video_DecDCToutCallbackFunc(void)
{
	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b5a64-0x800b5b7c.
#ifndef CTR_NATIVE
// part of PSYQ BSS
#define StCdIntrFlag *(u32 *)0x8009ebf8

	if (((V230.flags & 1) != 0) && (StCdIntrFlag != 0))
	{
		StCdInterrupt();

		StCdIntrFlag = 0;
	}
#else
	// NOTE(aalhendi): Native PsyCross does not map PSYQ BSS at 0x8009ebf8.
#endif

	uint32_t *ot = BreakDraw();

	LoadImage(&V230.slice, V230.out_Buf[V230.imgId]);

	/* update slice (rectangular strip) area to next one on the right */
	V230.slice.x += V230.slice.w;
	V230.imgId ^= 1;

	if (V230.frameCounter == V230.totalFrames)
	{
		V230.isDone = 1;
	}
	else
	{
		V230.frameCounter++;
		DecDCTout(V230.out_Buf[V230.imgId], V230.field32_0x58);
	}

	if (ot != 0)
	{
		DrawOTag(ot);
	}
}

void MM_Video_KickCD(CdlLOC *location)
{
	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b5b7c-0x800b5c8c.
	int iVar1;
	int CdlMode;
	u8 mode[4];

	if ((location != NULL) && (V230.ptrCdLoc != &V230.cdLocation2))
	{
		V230.field12_0x20 = 0;
		V230.ptrCdLoc = location;
	}

	switch (V230.field12_0x20)
	{
	case 0:
		// 2 = CdlSetloc
		iVar1 = CdControl(2, V230.ptrCdLoc, 0);
		if (iVar1 == 0)
			return;

		V230.field12_0x20 = 1;

		// do NOT break,
		// original code never quit here

	case 1:
		// CdlModeSpeed
		mode[0] = 0x80;

		// 0xe = CdlSetmode
		iVar1 = CdControl(0xe, mode, 0);
		if (iVar1 == 0)
			return;

		V230.field12_0x20 = 2;
		break;

	case 2:
		V230.field12_0x20 = 3;
		break;

	case 3:
		// CdlModeStream2|CdlModeSpeed
		CdlMode = 0x1a0;

		// scrapbook
		// if video contains audio
		if ((V230.flags & 2) != 0)
		{
			// CdlModeStream2|CdlModeSpeed|CdlModeRT
			CdlMode = 0x1e0;
		}

		V230.field12_0x20 = 0;

		iVar1 = CdRead2(CdlMode);
		if (iVar1 == 0)
			return;

		V230.ptrCdLoc = 0;
		break;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800b5c8c-0x800b615c.
void MM_Video_VLC_Decode(void)
{
	s16 oldDecodeState;
	int backloc;
	int result;
	uint32_t size;
	s16 freeSectors;
	s16 overSectors;
	uint32_t *sectorData;
	uint32_t *sectorHeader[2];
	int waitTime;
	CdlLOC *sectorLoc;

	waitTime = 10; // frames

	// free sectors and over sectors
	StRingStatus(&freeSectors, &overSectors);

	backloc = StGetBackloc(&V230.cdLocation2);

	oldDecodeState = V230.field9_0x1a;
	if ((V230.field9_0x1a == 1) && ((V230.RING_SIZE - (V230.RING_SIZE >> 2)) <= freeSectors))
	{
		V230.field14_0x24++;

		if (400 < V230.field14_0x24)
		{
			V230.field14_0x24 = 0;
			StClearRing();
			V230.field8_0x18 = 0;
			V230.frameCount = 0;
			V230.field19_0x30 = 0;
			V230.field20_0x34 = 0;
			V230.field0_0x0 = -1;
			V230.field1_0x4 = -1;
			V230.field2_0x8 = 0;
			V230.field13_0x22 = 0;
			V230.field14_0x24 = 0;
			V230.field21_0x38 = 0xffffffff;
			V230.field9_0x1a = oldDecodeState;

			MM_Video_KickCD(&V230.cdLocation1);
		}

		V230.drawNextFrame = 0;
		return;
	}

	V230.field14_0x24 = 0;

	// Scrapbook
	if (((V230.flags & 8) == 0) && (freeSectors < (V230.RING_SIZE >> 4)))
	{
		MM_Video_KickCD(&V230.cdLocation2);
	}

	if (backloc == V230.field20_0x34)
	{
		V230.field13_0x22++;
		if (0x40 < V230.field13_0x22)
		{
			V230.field13_0x22 = 0;
			V230.drawNextFrame = 0;
			StClearRing();
			V230.field19_0x30 = 0;
			V230.field0_0x0 = -1;
			V230.field1_0x4 = -1;
			V230.field2_0x8 = 0;
			V230.field14_0x24 = 0;
			V230.field20_0x34 = 0;
			V230.field13_0x22 = 0;
			V230.field21_0x38 = 0xffffffff;

			MM_Video_KickCD(&V230.cdLocation3);
		}
	}
	else
	{
		V230.field13_0x22 = 0;
	}

	V230.field9_0x1a = 0;

	// if reached end of video,
	// choose to loop or not loop
	if ((V230.field0_0x0 < 0) &&

	    // length of video
	    ((V230.numFrames <= backloc ||

	      (backloc < V230.field20_0x34))))
	{
		// scrapbook not track select,
		// if video is not looping
		if ((V230.flags & 4) == 0)
		{
			do
			{
				// 9 = CdlPause
				result = CdControl(9, 0, 0);
			} while (result == 0);
			// end of scrapbook
			V230.field8_0x18 = 1;
		}

		// track select, not scrapbook,
		// if video is looping
		else
		{
			V230.field21_0x38 = 0xffffffff;
			if (V230.field1_0x4 < 1)
			{
				MM_Video_KickCD(&V230.cdLocation1);

				if (backloc == V230.numFrames)
				{
					V230.field1_0x4 = CdPosToInt(&V230.cdLocation2);
				}
				else
				{
					V230.field0_0x0 = CdPosToInt(&V230.cdLocation2);
					V230.field0_0x0--;
					V230.field2_0x8 = 0;
				}
			}
			else
			{
				if (backloc != V230.numFrames)
				{
					result = CdPosToInt(&V230.cdLocation2);
					if (V230.field1_0x4 < result)
					{
						V230.field0_0x0 = CdPosToInt(&V230.cdLocation2);
						V230.field0_0x0 = V230.field0_0x0 + -1;
						V230.field2_0x8 = 0;

						MM_Video_KickCD(&V230.cdLocation1);
					}
					V230.field1_0x4 = -1;
				}
			}
		}
	}

	V230.field19_0x30 = V230.frameCount;
	V230.field20_0x34 = backloc;

LAB_800b5fec:

	// retrieve data with timeout (10 frames)
	do
	{
		result = StGetNext(&sectorData, sectorHeader);
		if (result == 0)
		{
			// sector->frameCount
			V230.frameCount = *(int *)((char *)sectorHeader[0] + 8);

			if (V230.frameCount == V230.field19_0x30)
			{
				StFreeRing(sectorData);
				goto LAB_800b5fec;
			}

			if (0 < V230.field0_0x0)
			{
				// sector->loc
				sectorLoc = (CdlLOC *)((char *)sectorHeader[0] + 0x1c);
				result = CdPosToInt(sectorLoc);

				waitTime = 10;

				if (V230.field0_0x0 <= result)
				{
					V230.field2_0x8 = 1;
					StFreeRing(sectorData);
					goto LAB_800b5fec;
				}
				if (V230.field2_0x8 == 1)
				{
					V230.field0_0x0 = -1;
					V230.field2_0x8 = 0;
					V230.field20_0x34 = backloc;
				}
			}

			size = DecDCTBufSize(sectorData);

			if (size <= V230.field25_0x48)
			{
				// sector->loc
				sectorLoc = (CdlLOC *)((char *)sectorHeader[0] + 0x1c);
				V230.cdLocation3 = *sectorLoc;

				// VLC Decode
				// last parameter is "VLC Table"
				DecDCTvlc2(sectorData, V230.in_Buf[V230.field10_0x1c], sdata->ptrVlcTable);

				// ready to draw next frame
				V230.drawNextFrame = 1;

				StFreeRing(sectorData);
				return;
			}
			V230.drawNextFrame = 0;
			StFreeRing(sectorData);
			return;
		}
		waitTime--;
	} while (waitTime != 0);

	V230.drawNextFrame = 0;
}

void MM_Video_StartStream(int param_1, int numFrames)
{
	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b615c-0x800b6260.
	V230.field3_0xc = 0;
	V230.field8_0x18 = 0;
	V230.field9_0x1a = 1;

	V230.isDone = 0;

	V230.frameCount = 0;
	V230.field19_0x30 = 0;
	V230.field20_0x34 = 0;
	V230.field0_0x0 = 0xffffffff;
	V230.field1_0x4 = 0xffffffff;
	V230.field2_0x8 = 0;
	V230.field13_0x22 = 0;
	V230.field14_0x24 = 0;
	V230.field21_0x38 = 0xffffffff;
	V230.drawNextFrame = 0;

	V230.numFrames = numFrames;

	// start streaming video
	CdIntToPos(param_1, &V230.cdLocation1);

	// V230.flags & 1 = IS_RGB24
	// next parameter (0) = START_FRAME
	StSetStream((V230.flags & 1), 0, 0xffffffff, 0, 0);

	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b6210-0x800b621c for video CD stream mode.
	CDSYS_SetMode_StreamData();

	// 800b6814 = Ring_Buf (mempack)
	StSetRing(V230.out_Buf[2], V230.RING_SIZE);

	StClearRing();

	V230.field12_0x20 = 0;

	V230.ptrCdLoc = &V230.cdLocation1;
}

void MM_Video_StopStream(void)
{
	// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b6260-0x800b62d8.
	int iVar1;

	iVar1 = CdDiskReady(1);
	if (iVar1 == 2)
	{
		do
		{
			// 9 = CdlPause
			iVar1 = CdControl(9, 0, 0);

		} while (iVar1 == 0);
	}

	StClearRing();

	StSetMask(1, 0, 0);

	CdDataCallback(0);

	CdReadyCallback(0);

	// Discontinue current decoding,
	// does not affect internal states (libref)
	DecDCTReset(1);

	V230.drawNextFrame = 0;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b62d8-0x800b64d4.
void MM_Video_AllocMem(u32 width, u16 height, u32 flags, int size, int param_5)
{
	char isRGB24;
	u32 uVar2;
	int iVar3;

	MEMPACK_PushState();

	// just in case
	width &= 0xffff;
	height &= 0xffff;

	V230.RING_SIZE = size;

	if (size < 1)
	{
		V230.RING_SIZE = 0x40;
	}

	isRGB24 = (flags & 1);

	iVar3 = (isRGB24) ? 3 : 2;

	V230.DCT_MODE = (u16)isRGB24;

	uVar2 = (((height - 1) >> 4) + 1) * 0x10;
	V230.totalFrames = (((width - 1) >> 4) + 1U) - 1;
	V230.imgId = 0;
	V230.field10_0x1c = 0;
	V230.field32_0x58 = (int)(iVar3 * 8 * uVar2) >> 1;
	V230.field25_0x48 = (int)(((width * iVar3) >> 1) * uVar2) >> (param_5 + 1U);
	V230.flags = flags;

	V230.out_Buf[0] = MEMPACK_AllocMem(V230.field32_0x58 << 3); //, OVR_230.s_SliceBuf);
	V230.out_Buf[1] = (uint32_t *)(((int)V230.out_Buf[0]) + V230.field32_0x58 * 4);

	V230.in_Buf[0] = MEMPACK_AllocMem(V230.field25_0x48 << 3); //, OVR_230.s_VlcBuf);
	V230.in_Buf[1] = (uint32_t *)(((int)V230.in_Buf[0]) + V230.field25_0x48 * 4);

	V230.out_Buf[2] = MEMPACK_AllocMem(V230.RING_SIZE << 0xb); //, OVR_230.s_RingBuf);

	V230.slice.x = 0;
	V230.slice.y = 0;
	V230.slice.w = (s16)(iVar3 << 3);
	V230.slice.h = height;

	// reinitialize everything
	DecDCTReset(0);

	DecDCTvlcSize2(V230.field25_0x48);
	EnterCriticalSection();
	DecDCToutCallback(&MM_Video_DecDCToutCallbackFunc);
	ExitCriticalSection();
}
#endif


// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b64d4-0x800b64f4.
void MM_Video_ClearMem(void)
{
	MEMPACK_PopState();
}

#ifndef CTR_NATIVE

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b64f4-0x800b6674.
u32 MM_Video_DecodeFrame(s16 offsetX, s16 offsetY)
{
	int iVar1;
	u32 boolDraw;

	iVar1 = CdDiskReady(1);
	if (V230.field3_0xc == 1)
	{
		if (iVar1 == 2)
		{
			V230.field3_0xc = 0;
			V230.drawNextFrame = 0;
			MM_Video_KickCD(&V230.cdLocation3);
			return 0;
		}
	}
	else
	{
		if (iVar1 == 0x10)
		{
			V230.field9_0x1a = 1;
			V230.field14_0x24 = 0;
			V230.field3_0xc = 1;
			V230.field20_0x34 = V230.frameCount - 1;
			StClearRing();
		}
	}
	if (V230.field3_0xc == 1)
	{
		V230.drawNextFrame = 0;
		boolDraw = 0;
	}
	else
	{
		if (V230.ptrCdLoc != 0)
		{
			MM_Video_KickCD(0);
		}

		MM_Video_VLC_Decode();

		// if value is zero, return zero,
		// not ready to draw
		boolDraw = V230.drawNextFrame;

		if (V230.drawNextFrame == 1)
		{
			V230.frameCounter = 0;

			V230.slice.x = offsetX;
			V230.slice.y = offsetY;

			// start decoding video
			DecDCTin(V230.in_Buf[V230.field10_0x1c], V230.DCT_MODE);

			V230.field10_0x1c ^= 1;

			// get result of decoding
			DecDCTout(V230.out_Buf[V230.imgId], V230.field32_0x58);

			// return 1, ready to draw
			boolDraw = (u32)V230.drawNextFrame;
		}
	}
	return boolDraw;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 PSX path 0x800b6674-0x800b67ac.
u32 MM_Video_CheckIfFinished(int param_1)
{
	char bVar1;
	u32 uVar2;
	int iVar3;
	int local_20;
	int local_1c;

#ifdef CTR_NATIVE
	// NOTE(aalhendi): Native compiler optimizes this delay loop differently;
	// retail PSX uses 40000.
	local_20 = 90000;
#else
	local_20 = 40000;
#endif

	local_1c = 0x28;

	bVar1 = false;

	if (V230.drawNextFrame == 0)
	{
		uVar2 = 0;
	}
	else
	{
		do
		{
			if ((param_1 == 1) && (local_1c--, local_1c == 0))
			{
				iVar3 = CdDiskReady(1);

				if (iVar3 == 0x10)
				{
					bVar1 = true;
					V230.isDone = 1;
				}
				else
				{
					local_1c = 0x28;
				}
			}

			local_20--;

			if (local_20 == 0)
			{
				V230.isDone = 1;
			}

		} while (!V230.isDone);

		do
		{
			iVar3 = IsIdleGPU(10000);

		} while (iVar3 != 0);

		V230.isDone = 0;

		V230.drawNextFrame = 0;

		if ((!bVar1) && (V230.frameCounter != V230.totalFrames))
		{
			// Discontinue current decoding,
			// does not affect internal states (libref)
			DecDCTReset(1);
		}

		// end of scrapbook
		uVar2 = (u32)V230.field8_0x18;
	}
	return uVar2;
}
#endif
