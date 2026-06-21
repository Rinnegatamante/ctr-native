#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3dd8-0x800b3f88.
void AH_MaskHint_Start(s16 hintId, u16 bool_interruptWarppad)
{
	int iVar3;
	int bitIndex;
	struct Driver *d;

	// copy parameters
	D232.maskWarppadBoolInterrupt = bool_interruptWarppad;
	D232.maskHintID = hintId;

	sdata->boolDraw3D_AdvMask = 1;

	struct AdvProgress *adv = &sdata->advProgress;
	bitIndex = (int)hintId + ADV_REWARD_FIRST_HINT;
	UNLOCK_ADV_BIT(adv->rewards, bitIndex);

	// If this is "welcome to adventure arena"
	if (hintId == ADV_MASK_HINT_ID_WELCOME_TO_ARENA)
	{
		// "Using a Warppad" and "Map Information"
		UNLOCK_ADV_BIT(adv->rewards, ADV_REWARD_HINT_USING_WARP_PAD);
		UNLOCK_ADV_BIT(adv->rewards, ADV_REWARD_HINT_MAP_INFORMATION);
	}

	d = sdata->gGT->drivers[0];
	d->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_FreezeEndEvent_Init;

	// If Aku / Uka model pointer is nullptr
	if (sdata->modelMaskHints3D == NULL)
	{
		LOAD_TalkingMask(LOAD_GetAdvPackIndex(), (VehPickupItem_MaskBoolGoodGuy(d) & 0xffff) == 0);

		// 3.0s to spawn mask
		D232.maskSpawnFrame = 90;
	}

	// if model is not nullptr
	else
	{
		// 0.667s to spawn mask
		D232.maskSpawnFrame = 20;
	}

	iVar3 = (bool_interruptWarppad & 1) * 3;

	s16 *input = &D232.maskVars[0];

	D232.maskOffsetPos.x = input[iVar3 + 0];
	D232.maskOffsetPos.y = input[iVar3 + 1];
	D232.maskOffsetPos.z = input[iVar3 + 2];

	D232.maskOffsetRot.x = input[iVar3 + 6];
	D232.maskOffsetRot.y = input[iVar3 + 7];
	D232.maskOffsetRot.z = input[iVar3 + 8];

	for (int i = 0; i < 3; i++)
	{
		// 4 bytes for 4 volumes
		// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b3f3c-0x800b3f54 for mask-hint volume backup.
		D232.audioBackup[i] = howl_VolumeGet(i);
	}

	return;
}
