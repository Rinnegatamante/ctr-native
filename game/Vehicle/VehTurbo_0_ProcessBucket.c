#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80069284-0x80069370.
void VehTurbo_ProcessBucket(struct Thread *turboThread)
{
	while (turboThread != NULL)
	{
		struct Instance *primaryInst = turboThread->inst;
		struct Turbo *turbo = (struct Turbo *)turboThread->object;
		struct Instance *secondaryInst = turbo->inst;
		struct Instance *driverInst = turbo->driver->instSelf;

		struct InstDrawPerPlayer *primary = INST_GETIDPP(primaryInst);
		struct InstDrawPerPlayer *secondary = INST_GETIDPP(secondaryInst);
		struct InstDrawPerPlayer *driver = INST_GETIDPP(driverInst);

		for (int i = 0; i < sdata->gGT->numPlyrCurrGame; i++)
		{
			if ((driver->instFlags & PUSHBUFFER_EXISTS) == 0)
			{
				u32 driverDrawFlag = driver->instFlags | ~DRAW_SUCCESSFUL;

				secondary->instFlags &= driverDrawFlag;
				primary->instFlags &= driverDrawFlag;

				secondary->unkE4 = driver->unkE4;
				primary->unkE4 = driver->unkE4;
				secondary->unkE8 = driver->unkE8;
				primary->unkE8 = driver->unkE8;

				secondary->depthOffset[0] = driver->depthOffset[0];
				primary->depthOffset[0] = driver->depthOffset[0];
				secondary->depthOffset[1] = driver->depthOffset[1];
				primary->depthOffset[1] = driver->depthOffset[1];
			}

			primary++;
			secondary++;
			driver++;
		}

		turboThread = turboThread->siblingThread;
	}
}
