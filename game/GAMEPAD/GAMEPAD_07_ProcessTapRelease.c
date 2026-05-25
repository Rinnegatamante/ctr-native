#include <common.h>

// Writes all gamepad variables
// for Tap and Release, based on Hold,
// also maps joysticks onto buttons

void GAMEPAD_ProcessTapRelease(struct GamepadSystem *gGamepads)
{
	int numConnected = gGamepads->numGamepadsConnected;
	if (numConnected == 0)
		return;

	char cVar1;
	cVar1 = sdata->unkPadSetActAlign[6];

	struct GamepadBuffer *pad;
	struct ControllerPacket *ptrControllerPacket;


	for (pad = &gGamepads->gamepad[0]; pad < &gGamepads->gamepad[numConnected]; pad++)
	{
		ptrControllerPacket = pad->ptrControllerPacket;

		// if pointer is invalid
		if (ptrControllerPacket == NULL)
		{
			// erase tap and release
			pad->buttonsTapped = 0;
			pad->buttonsReleased = 0;
		}
		else
		{
			if (cVar1 != 0)
			{
				if (pad->stickLX < 0x20)
				{
					pad->buttonsHeldCurrFrame |= BTN_LEFT;
				}

				else if (0xe0 < pad->stickLX)
				{
					pad->buttonsHeldCurrFrame |= BTN_RIGHT;
				}

				if (pad->stickLY < 0x20)
				{
					pad->buttonsHeldCurrFrame |= BTN_UP;
				}

				else if (pad->stickLY > 0xe1)
				{
					pad->buttonsHeldCurrFrame |= BTN_DOWN;
				}
			}

			// tapped
			pad->buttonsTapped = ~pad->buttonsHeldPrevFrame & pad->buttonsHeldCurrFrame;

			// released
			pad->buttonsReleased = pad->buttonsHeldPrevFrame & ~pad->buttonsHeldCurrFrame;
		}
	}
}
