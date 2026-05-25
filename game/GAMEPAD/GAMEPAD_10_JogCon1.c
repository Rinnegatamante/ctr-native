#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800263a0-0x800263fc
void GAMEPAD_JogCon1(struct Driver *d, char val, u16 timeMS)
{
	// if AI
	if ((d->actionsFlagSet & 0x100000) != 0)
		return;

	struct GamepadBuffer *gb = &sdata->gGamepads->gamepad[d->driverID];

	if ((gb->unk45 & 0xf) > (val & 0xf))
		return;

	gb->unk45 = val;
	gb->unk46 = timeMS;
}
