#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800347d0-0x80034874.
// Dont get confused, packID is DECOMP_LOAD_GetAdvPackIndex(),
// which gives the pack of the hub you're NOT on, because the
// game does 3-GetAdvPackIndex to load the hub you ARE on
void DECOMP_LOAD_TalkingMask(int packID, int maskID)
{
	sdata->modelMaskHints3D = 0;

	// invalidate alternative-hub, because
	// the mask will load in that level's RAM
	sdata->gGT->levID_in_each_mempack[packID] = -1;

	// Swap to pack of hub you're NOT on,
	// wipe the pack to reload the new MASK
	MEMPACK_SwapPacks(packID);
	MEMPACK_ClearLowMem();

	int offset = maskID * 4 + (packID - 1) * 2;

	// NOTE(aalhendi): Retail passes legacy VRAM type 3 with no callback. Native
	// uses bit-flag load types, so the VRAM upload callback stays explicit.
	DECOMP_LOAD_AppendQueue(0, LT_SETVRAM, BI_UKAHEAD + offset, NULL, DECOMP_LOAD_VramFileCallback);

	DECOMP_LOAD_AppendQueue(0, LT_GETADDR, BI_UKAHEAD + offset + 1, NULL, DECOMP_LOAD_Callback_MaskHints3D);
}
