#include <common.h>

void MainFrame_ResetDB(struct GameTracker *gGT)
{
	struct GameTracker *psVar1;
	int iVar2;
	u_long *puVar3;
	int iVar4;
	struct DB *db;
	int otSwapchainDB;

// Dont use this in DECOMP until drivers->stepFlagSet is written.
// NOTE(aalhendi): ctr-native uses the REBUILD_PS1 code path, but still needs
// the retail hub swap pump for cutscene opcode 0xf level transitions.
#if !defined(REBUILD_PS1) || defined(CTR_NATIVE)
	// check if new adv hub should be loaded,
	// this was a random place for ND to put it
	DECOMP_LOAD_Hub_Main(sdata->ptrBigfile1);
#endif

#ifndef REBUILD_PC
	gGT->swapchainIndex = 1 - gGT->swapchainIndex;
#endif

	gGT->backBuffer = &gGT->db[gGT->swapchainIndex];
	gGT->frameTimer_MainFrame_ResetDB++;

	otSwapchainDB = (int)gGT->otSwapchainDB[gGT->swapchainIndex];

	db = gGT->backBuffer;
	*(u8 *)&db->unk_primMemRelated = 0;
	db->primMem.curr = db->primMem.start;
	db->primMem.unk1 = 0;
	db->otMem.curr = db->otMem.start;

// These functions literally do nothing,
// even in the OG game, they're empty
#if 0
	CTR_EmptyFunc_MainFrame_ResetDB();
	DecalGlobal_EmptyFunc_MainFrame_ResetDB();
#endif

// Test for persistent storage,
// This requires duckstation overclock of 10x
#if 0
	memset(gGT->backBuffer->primMem.start, 0, gGT->backBuffer->primMem.size);
#endif

	ClearOTagR((u32 *)otSwapchainDB, gGT->numPlyrCurrGame << 10 | 6);

	for (iVar4 = 0; iVar4 < gGT->numPlyrCurrGame; iVar4++)
	{
		iVar2 = (u32)(u8)gGT->numPlyrCurrGame - iVar4;

		gGT->pushBuffer[iVar4].ptrOT = (u_long *)((int)otSwapchainDB + (gGT->numPlyrCurrGame - iVar4 - 1) * 0x1000 + 0x18);
	}

	for (iVar4; iVar4 < 4; iVar4++)
	{
		// but why?
		gGT->pushBuffer[iVar4].ptrOT = (u_long *)((int)otSwapchainDB + 3 * 0x1000 + 0x18);
	}

	puVar3 = (u_long *)((int)otSwapchainDB + 4);
	gGT->pushBuffer_UI.ptrOT = puVar3;
	db->otMem.startPlusFour = puVar3;
	return;
}
