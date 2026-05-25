#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 exe 0x80035d70-0x80035e20.
int MainFrame_HaveAllPads(s16 numPlyrNextGame)
{
	// if game is not loading
	if (sdata->Loading.stage == -1)
	{
		struct GamepadBuffer *gb = &sdata->gGamepads->gamepad[0];

		if (numPlyrNextGame == 0)
			return 0;

		for (int i = 0; i < numPlyrNextGame; i++)
		{
			struct ControllerPacket *packet = gb->ptrControllerPacket;

			if (packet == NULL)
				return 0;
			if (packet->plugged != PLUGGED)
				return 0;

			gb++;
		}
	}

	return 1;
}
