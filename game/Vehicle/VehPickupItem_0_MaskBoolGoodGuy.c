#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80064be4-0x80064c38.
int VehPickupItem_MaskBoolGoodGuy(struct Driver *d)
{
	int charID;
	charID = data.characterIDs[d->driverID];

	// Crash, Coco, Pura, Polar, Penta
	u32 maskBits = 0x20c9;

	return (maskBits >> charID) & 1;
}
