#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ada4c-0x800adae4.
int MM_Characters_GetNextDriver(s16 dpad, char characterID)
{
	char nextDriver;
	s16 unlocked;
	char newDriver;

	nextDriver = D230.csm_Active[characterID].indexNext[dpad];
	unlocked = D230.csm_Active[nextDriver].unlockFlags;

	// set new driver to the driver
	// you'd get when pressing Up button
	newDriver = nextDriver;

	if (
	    // if desired driver is not unlocked by default
	    (unlocked != -1) &&

	    (((sdata->gameProgress.unlocks[unlocked >> 5] >> (unlocked & 0x1f)) & 1) == 0))
	{
		// set new driver to the driver you already have
		newDriver = characterID;
	}

	// return new driver
	return newDriver;
}
