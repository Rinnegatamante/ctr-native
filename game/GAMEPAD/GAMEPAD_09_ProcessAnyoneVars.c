#include <common.h>

#ifdef CTR_INTERNAL
// NOTE(aalhendi): Internal GDB input shim for runtime probes.
// gCtrDebugPadTap is one-frame input; gCtrDebugPadHeld stays pressed until reset.
// Example route, main menu -> Time Trial:
//   # Select Adventure/Time Trial row from main menu, then confirm each menu.
//   set D230.menuMainMenu.rowSelected=1
//   set gCtrDebugPadTap=0x10
//   continue
//   set gCtrDebugPadTap=0x10
//   continue
//   set gCtrDebugPadTap=0x10
//   continue
volatile int gCtrDebugPadHeld = 0;
volatile int gCtrDebugPadTap = 0;
#endif

/// @brief Main gamepad processing function. Polls every connected gamepad and generates global state flags.
/// @param gGamepads - gamepad input system
void GAMEPAD_ProcessAnyoneVars(struct GamepadSystem *gGamepads)
{
	struct GamepadBuffer *pad;

	// process gamepads
	GAMEPAD_ProcessHold(gGamepads);
	GAMEPAD_ProcessSticks(gGamepads);
	GAMEPAD_ProcessTapRelease(gGamepads);
	GAMEPAD_ProcessMotors(gGamepads);

	// These are used to see if any button is pressed by anyone
	// during this frame. Reset them all to zero
	gGamepads->anyoneHeldCurr = 0;
	gGamepads->anyoneTapped = 0;
	gGamepads->anyoneReleased = 0;
	gGamepads->anyoneHeldPrev = 0;

	// foreach connected gamepad
	for (int i = 0; i < gGamepads->numGamepadsConnected; i++)
	{
		// get gamepad
		pad = &gGamepads->gamepad[i];

		// update global system flag
		gGamepads->anyoneHeldCurr |= pad->buttonsHeldCurrFrame;
		gGamepads->anyoneTapped |= pad->buttonsTapped;
		gGamepads->anyoneReleased |= pad->buttonsReleased;
		gGamepads->anyoneHeldPrev |= pad->buttonsHeldPrevFrame;
	}

#ifdef CTR_INTERNAL
	if (gGamepads->numGamepadsConnected > 0)
	{
		pad = &gGamepads->gamepad[0];

		if (gCtrDebugPadHeld != 0)
		{
			pad->buttonsHeldCurrFrame |= gCtrDebugPadHeld;
			gGamepads->anyoneHeldCurr |= gCtrDebugPadHeld;
		}

		if (gCtrDebugPadTap != 0)
		{
			pad->buttonsTapped |= gCtrDebugPadTap;
			gGamepads->anyoneTapped |= gCtrDebugPadTap;
			gGamepads->anyoneHeldCurr |= gCtrDebugPadTap;
			gCtrDebugPadTap = 0;
		}
	}
#endif
}
