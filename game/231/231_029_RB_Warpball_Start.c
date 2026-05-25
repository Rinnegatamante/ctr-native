#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae778-0x800ae7dc.
void RB_Warpball_Start(struct TrackerWeapon *tw)
{
	tw->ptrNodeCurr = RB_Warpball_NewPathNode(tw->ptrNodeCurr, tw->driverTarget);
	tw->ptrNodeNext = RB_Warpball_NewPathNode(tw->ptrNodeCurr, tw->driverTarget);
	return;
}
