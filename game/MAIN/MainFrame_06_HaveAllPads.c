#include <common.h>

int MainFrame_HaveAllPads(s16 numPlyrNextGame)
{
	// if game is not loading
	if (sdata->Loading.stage == -1)
	{
		struct GamepadBuffer *gb = &sdata->gGamepads->gamepad[0];

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
