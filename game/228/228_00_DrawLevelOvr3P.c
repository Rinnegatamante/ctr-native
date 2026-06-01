#include <common.h>

typedef int (*Ovr228BucketDispatch)(u32 handlerAddress, void *bucketValue, struct PushBuffer *pb, struct mesh_info *mesh, struct PrimMem *primMem,
                                    const int *visFaceList);
typedef int (*Ovr228ClipConsumer)(struct PushBuffer *pb, struct PrimMem *primMem, u8 *clipCursor, int playerIndex);

static const struct OverlayRDATA_228_BucketSetupRecord *Ovr228_800a0fb8_FindBucketSetupRecord(u32 setupAddress, int *setupIndex)
{
	for (int i = 0; i < OVR228_BUCKET_COUNT; i++)
	{
		u32 recordAddress = OVR228_RDATA_BUCKET_SETUP_BASE + (u32)(i * sizeof(R228.bucketSetups[0]));

		if (recordAddress == setupAddress)
		{
			if (setupIndex != NULL)
				*setupIndex = i;

			return &R228.bucketSetups[i];
		}
	}

	return NULL;
}

static const u32 *Ovr228_800a0fb8_GetCopySource(const struct OverlayRDATA_228_BucketSetupRecord *setup, int setupIndex,
                                                const struct OverlayRDATA_228_BucketSetupCopy *copy)
{
	u32 recordAddress = OVR228_RDATA_BUCKET_SETUP_BASE + (u32)(setupIndex * sizeof(R228.bucketSetups[0]));
	u32 copy0Address = recordAddress + OFFSETOF(struct OverlayRDATA_228_BucketSetupRecord, copy0);
	u32 copy1Address = recordAddress + OFFSETOF(struct OverlayRDATA_228_BucketSetupRecord, copy1);

	if (copy->sourceAddress == copy0Address)
		return setup->copy0;

	if (copy->sourceAddress == copy1Address)
		return setup->copy1;

	return NULL;
}

static void Ovr228_800a0fb8_CopyScratchWords(const struct OverlayRDATA_228_BucketSetupRecord *setup, int setupIndex,
                                             const struct OverlayRDATA_228_BucketSetupCopy *copy)
{
	const u32 *source = Ovr228_800a0fb8_GetCopySource(setup, setupIndex, copy);
	u32 *scratch = CTR_SCRATCHPAD_PTR(u32, copy->scratchOffset);

	if (source == NULL)
		return;

	for (u32 i = 0; i <= copy->loopCounter; i++)
		scratch[i] = source[i];
}

static void Ovr228_800a0fb8_ApplyBucketSetup(u32 setupAddress, u32 handlerAddress)
{
	int setupIndex = -1;
	const struct OverlayRDATA_228_BucketSetupRecord *setup = Ovr228_800a0fb8_FindBucketSetupRecord(setupAddress, &setupIndex);

	if (setup != NULL)
	{
		for (int i = 0; i < 2; i++)
		{
			const struct OverlayRDATA_228_BucketSetupCopy *copy = &setup->copies[i];

			if (copy->loopCounter == 0)
				break;

			Ovr228_800a0fb8_CopyScratchWords(setup, setupIndex, copy);
		}
	}

	*CTR_SCRATCHPAD_PTR(u32, 0x74) = handlerAddress;
}

static void Ovr228_800a0d68_CopyScratchInitTable(void)
{
	u32 *scratch = CTR_SCRATCHPAD_PTR(u32, 0xec);

	for (int i = 0; i < OVR228_SCRATCH_INIT_WORD_COUNT; i++)
		scratch[i] = R228.scratchInitTable[i];
}

static void Ovr228_800a8e08_CopyClipRecordJumpTable(void)
{
	u32 *clipRecordJumpTable = CTR_SCRATCHPAD_PTR(u32, 0x240);

	for (int i = 0; i < OVR228_CLIP_RECORD_JUMP_WORD_COUNT; i++)
		clipRecordJumpTable[i] = R228.clipRecordJumpTable[i];
}

static void Ovr228_800a1014_SetViewportContext(struct PushBuffer *pb, const int *visFaceList, u8 *clipStart, u8 *clipCursor,
                                               struct QuadBlock **renderedOverflowBase)
{
	*CTR_SCRATCHPAD_PTR(u32, 0x64) = (u32)(uintptr_t)renderedOverflowBase;
	*CTR_SCRATCHPAD_PTR(u32, 0x10) = (u32)(uintptr_t)clipCursor;
	*CTR_SCRATCHPAD_PTR(u32, 0xc8) = (u32)(uintptr_t)visFaceList;
	DrawLevelOvr1P_SetClipRecordStart(clipStart);
	DrawLevelOvr1P_SetRenderedOverflowBase(renderedOverflowBase);
	Ovr226_800a0d34_SetEntryGteAndCameraScratch(pb);
}

static void Ovr228_ClearRenderedOverflowBase(int playerIndex)
{
	struct QuadBlock **renderedOverflowBase = (struct QuadBlock **)data.ptrRenderedQuadblockDestination_forEachPlayer[playerIndex];

	if (renderedOverflowBase != NULL)
		*renderedOverflowBase = NULL;
}

static int Ovr228_DrawViewportBucket(struct DrawLevelOvr1PRenderList *renderList, s32 renderListOffset, struct PushBuffer *pb, struct mesh_info *mesh,
                                     struct PrimMem *primMem, const int *visFaceList, u8 **clipCursor, int playerIndex, int applySetup,
                                     Ovr228BucketDispatch dispatch, int *didDispatch)
{
	u32 bucketIndex = (u32)renderListOffset / sizeof(u32);
	void *bucketValue = DrawLevelOvr1P_GetRenderListField(renderList, renderListOffset);
	u32 setupAddress = R228.bucketSetupAddresses[bucketIndex];
	u32 handlerAddress = R228.bucketHandlerAddresses[bucketIndex];
	struct QuadBlock **renderedOverflowBase = (struct QuadBlock **)data.ptrRenderedQuadblockDestination_forEachPlayer[playerIndex];

	*didDispatch = 0;

	if (bucketValue == NULL)
	{
		Ovr228_ClearRenderedOverflowBase(playerIndex);
		return 1;
	}

	if (applySetup)
		Ovr228_800a0fb8_ApplyBucketSetup(setupAddress, handlerAddress);

	Ovr228_800a1014_SetViewportContext(pb, visFaceList, data.PtrClipBuffer[playerIndex], *clipCursor, renderedOverflowBase);
	if (!dispatch(handlerAddress, bucketValue, pb, mesh, primMem, visFaceList))
		return 0;

	*clipCursor = DrawLevelOvr1P_GetClipRecordCursor();
	*didDispatch = 1;
	return 1;
}

static int Ovr228_800a0d9c_DispatchBucketTable(struct DrawLevelOvr1PRenderList *renderLists, struct PushBuffer *pushBuffers, struct mesh_info *mesh,
                                               struct PrimMem *primMem, const int *visFaceList0, const int *visFaceList1, const int *visFaceList2,
                                               u8 **clipCursors, Ovr228BucketDispatch dispatch)
{
	for (s32 renderListOffset = 0x1c; renderListOffset >= 0; renderListOffset -= (s32)sizeof(u32))
	{
		int setupApplied = 0;
		int didDispatch = 0;

		*CTR_SCRATCHPAD_PTR(u32, 0x34) = (u32)renderListOffset;

		if (!Ovr228_DrawViewportBucket(&renderLists[0], renderListOffset, &pushBuffers[0], mesh, primMem, visFaceList0, &clipCursors[0], 0, 1, dispatch,
		                               &didDispatch))
			return 0;
		setupApplied |= didDispatch;

		if (!Ovr228_DrawViewportBucket(&renderLists[1], renderListOffset, &pushBuffers[1], mesh, primMem, visFaceList1, &clipCursors[1], 1, !setupApplied,
		                               dispatch, &didDispatch))
			return 0;
		setupApplied |= didDispatch;

		if (!Ovr228_DrawViewportBucket(&renderLists[2], renderListOffset, &pushBuffers[2], mesh, primMem, visFaceList2, &clipCursors[2], 2, !setupApplied,
		                               dispatch, &didDispatch))
			return 0;
	}

	return 1;
}

static int Ovr228_ConsumeClipRecordsForViewport(struct PushBuffer *pb, struct PrimMem *primMem, u8 *clipCursor, int playerIndex, Ovr228ClipConsumer consume)
{
	u8 *start = data.PtrClipBuffer[playerIndex];

	DrawLevelOvr1P_SetClipRecordStart(start);
	DrawLevelOvr1P_SetClipRecordCursor(clipCursor);
	return consume(pb, primMem, clipCursor, playerIndex);
}

static int Ovr228_UnportedBucketDispatch(u32 handlerAddress, void *bucketValue, struct PushBuffer *pb, struct mesh_info *mesh, struct PrimMem *primMem,
                                         const int *visFaceList)
{
	(void)handlerAddress;
	(void)bucketValue;
	(void)pb;
	(void)mesh;
	(void)primMem;
	(void)visFaceList;

	// NOTE(aalhendi): 0x800a10c4+ bucket families are outside this pass.
	// Fail closed if this audit-only entry is called without real handlers.
	return 0;
}

static int Ovr228_UnportedClipConsumer(struct PushBuffer *pb, struct PrimMem *primMem, u8 *clipCursor, int playerIndex)
{
	(void)pb;
	(void)primMem;
	(void)clipCursor;
	(void)playerIndex;

	// NOTE(aalhendi): 0x800a81bc+ terminal core is outside this pass.
	// Fail closed if this audit-only entry is called without a real consumer.
	return 0;
}

static int Ovr228_800a0cbc_EntryWithCallbacks(void *LevRenderList, struct PushBuffer *pb, struct BSP *bspList, struct PrimMem *primMem, void *VisMem10,
                                              void *VisMem14, void *VisMem18, void *waterEnvMap, Ovr228BucketDispatch dispatch, Ovr228ClipConsumer consume)
{
	struct DrawLevelOvr1PRenderList *renderLists = LevRenderList;
	struct mesh_info *mesh = (struct mesh_info *)bspList;
	const int *visFaceList0 = VisMem10;
	const int *visFaceList1 = VisMem14;
	const int *visFaceList2 = VisMem18;
	u8 *clipCursors[3];
	u32 hostStackAnchor;

	// NOTE(aalhendi): ASM-audited against NTSC-U 926 228 entry/setup
	// 0x800a0cbc-0x800a10c4. The public 3P renderer still uses the 226
	// fallback until the downstream 228 bucket and terminal families are owned.
	*CTR_SCRATCHPAD_PTR(u32, 0x38) = (u32)(uintptr_t)&hostStackAnchor;
	*CTR_SCRATCHPAD_PTR(u32, 0xcc) = (u32)(uintptr_t)VisMem10;
	if (VisMem10 == NULL)
		return 1;

	*CTR_SCRATCHPAD_PTR(u32, 0xd0) = (u32)(uintptr_t)VisMem14;
	if (VisMem14 == NULL)
		return 1;

	*CTR_SCRATCHPAD_PTR(u32, 0xd4) = (u32)(uintptr_t)VisMem18;
	if (VisMem18 == NULL)
		return 1;

	*CTR_SCRATCHPAD_PTR(u32, 0x88) = (u32)(uintptr_t)waterEnvMap;
	*CTR_SCRATCHPAD_PTR(u32, 0x30) = (u32)(uintptr_t)primMem->end;

	if (mesh->ptrQuadBlockArray == NULL)
		return 1;

	clipCursors[0] = data.PtrClipBuffer[0];
	clipCursors[1] = data.PtrClipBuffer[1];
	clipCursors[2] = data.PtrClipBuffer[2];

	*CTR_SCRATCHPAD_PTR(u32, 0xdc) = (u32)(uintptr_t)&pb[0];
	*CTR_SCRATCHPAD_PTR(u32, 0xe0) = (u32)(uintptr_t)&pb[1];
	*CTR_SCRATCHPAD_PTR(u32, 0xe4) = (u32)(uintptr_t)&pb[2];
	*CTR_SCRATCHPAD_PTR(u32, 0x0) = (u32)(uintptr_t)clipCursors[0];
	*CTR_SCRATCHPAD_PTR(u32, 0x4) = (u32)(uintptr_t)clipCursors[1];
	*CTR_SCRATCHPAD_PTR(u32, 0x8) = (u32)(uintptr_t)clipCursors[2];

	DrawLevelOvr1P_SetPrimReserveBias(0);
	DrawLevelOvr1P_SetListHandlersSeedRenderedCursor(0);
	Ovr226_800a0dc4_ClearProjectedScratch();
	Ovr228_800a0d68_CopyScratchInitTable();
	*CTR_SCRATCHPAD_PTR(u32, 0x60) = (u32)(uintptr_t)LevRenderList;

	if (!Ovr228_800a0d9c_DispatchBucketTable(renderLists, pb, mesh, primMem, visFaceList0, visFaceList1, visFaceList2, clipCursors, dispatch))
		return 0;

	*CTR_SCRATCHPAD_PTR(u32, 0x0) = (u32)(uintptr_t)clipCursors[0];
	*CTR_SCRATCHPAD_PTR(u32, 0x4) = (u32)(uintptr_t)clipCursors[1];
	*CTR_SCRATCHPAD_PTR(u32, 0x8) = (u32)(uintptr_t)clipCursors[2];

	Ovr228_800a8e08_CopyClipRecordJumpTable();
	if (!Ovr228_ConsumeClipRecordsForViewport(&pb[0], primMem, clipCursors[0], 0, consume))
		return 0;

	if (!Ovr228_ConsumeClipRecordsForViewport(&pb[1], primMem, clipCursors[1], 1, consume))
		return 0;

	if (!Ovr228_ConsumeClipRecordsForViewport(&pb[2], primMem, clipCursors[2], 2, consume))
		return 0;

	return 1;
}

int Ovr228_800a0cbc_Entry(void *LevRenderList, struct PushBuffer *pb, struct BSP *bspList, struct PrimMem *primMem, void *VisMem10, void *VisMem14,
                          void *VisMem18, void *waterEnvMap)
{
	return Ovr228_800a0cbc_EntryWithCallbacks(LevRenderList, pb, bspList, primMem, VisMem10, VisMem14, VisMem18, waterEnvMap, Ovr228_UnportedBucketDispatch,
	                                          Ovr228_UnportedClipConsumer);
}
