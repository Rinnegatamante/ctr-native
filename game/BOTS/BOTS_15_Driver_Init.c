#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80017164-0x80017318.
struct Driver *BOTS_Driver_Init(int driverID)
{
	struct Thread *t;
	struct Driver *d;

	char initialNavPathIndex = sdata->driver_pathIndexIDs[driverID];
	char navPathIndex;
	s16 navPathPointsCount; // = sdata->NavPath_ptrHeader[navPathIndex]->numPoints;

	navPathIndex = initialNavPathIndex;
	while (1)
	{
		navPathPointsCount = sdata->NavPath_ptrHeader[navPathIndex]->numPoints;
		if (1 < navPathPointsCount)
			break; // success

		navPathIndex--;

		// If subtracted below zero,
		// go back to highest index (2)
		if (navPathIndex < 0)
			navPathIndex = 2;

		// If all 3 are checked, quit
		if (navPathIndex == initialNavPathIndex)
			return NULL;
	}

	// path data found
	t = PROC_BirthWithObject(
	    // creation flags
	    SIZE_RELATIVE_POOL_BUCKET(0x62c, NONE, LARGE, ROBOT),

	    BOTS_ThTick_Drive, // behavior
	    0,                 //"robotcar",	// debug name
	    0                  // thread relative
	);

	d = t->object;
	memset(d, 0x0, 0x62c);
	VehBirth_NonGhost(t, driverID);
	sdata->gGT->drivers[driverID] = d;
	t->modelIndex = DYNAMIC_ROBOT_CAR;

	d->botData.botPath = navPathIndex;
	d->botData.botNavFrame = sdata->NavPath_ptrNavFrameArray[navPathIndex];
	d->actionsFlagSet |= 0x100000;
	LIST_AddFront(&sdata->navBotList[navPathIndex], (struct Item *)(&d->botData));

	sdata->gGT->numBotsNextGame++;
	BOTS_GotoStartingLine(d);
	return d;
}
