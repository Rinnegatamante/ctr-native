#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800ae0bc-0x800ae274.
void MM_Characters_SetMenuLayout(void)
{
	u16 unlocked;
	char expand;
	int iVar3;
	int i;
	int numPlyrNextGame;

	expand = 0;

	// By default, draw "Select character" in 3P menu
	D230.isRosterExpanded = 0;

	numPlyrNextGame = sdata->gGT->numPlyrNextGame;

	iVar3 = numPlyrNextGame - 1;

	// original game
#define NUM_ICONS 0xF

	// Loop through bottom characters,
	// if any are unlocked, use expanded
	for (i = 0xc; i < NUM_ICONS; i++)
	{
		// OG game code
		unlocked = D230.csm_1P2P[i].unlockFlags;

		if (((sdata->gameProgress.unlocks[unlocked >> 5] >> (unlocked & 0x1f)) & 1) != 0)
		{
			expand = 1;
			break;
		}
	}

	if (
	    // if 1P2P (0 or 1)
	    (iVar3 < 2) &&

	    // if very few characters are unlocked
	    (!expand))
	{
		// layout [4] and [5] for 1P2P without expansion
		iVar3 += 4;
	}

	D230.isRosterExpanded = expand;

	D230.characterSelectIconLayout = iVar3;

	D230.csm_instPos.y = D230.driverPosY[iVar3];
	D230.csm_instPos.z = D230.driverPosZ[iVar3];

	D230.characterSelect_sizeX = D230.windowW[iVar3];
	D230.characterSelect_sizeY = D230.windowH[iVar3];

	D230.characterSelect_ptrWindowXY = D230.ptrSelectWindowPos[iVar3];

	D230.csm_Active = D230.ptrCsmArr[iVar3];

	D230.textPos = D230.textPosArr[iVar3];

	D230.ptrTransitionMeta = D230.ptr_transitionMeta_csm[numPlyrNextGame - 1];

	return;
}
