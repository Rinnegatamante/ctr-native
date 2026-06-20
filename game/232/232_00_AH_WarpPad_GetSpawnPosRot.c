#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abafc-0x800abbdc.
s16 *AH_WarpPad_GetSpawnPosRot(s16 *posData)
{
	struct Thread *t;
	struct GameTracker *gGT;
	struct Instance *inst;
	struct InstDef *instDef;

	gGT = sdata->gGT;
	t = gGT->threadBuckets[WARPPAD].thread;

	// check all warppads
	while (1)
	{
		// if can't find a warppad
		if (t == 0)
		{
			// quit
			return 0;
		}

		// if warppad found that matches level exited
		if (((struct WarpPad *)t->object)->levelID == gGT->prevLEV)
		{
			// end loop
			break;
		}

		t = t->siblingThread;
	}

	inst = t->inst;
	instDef = inst->instDef;

	posData[0] = inst->matrix.t[0] + ((MATH_Cos(instDef->rot.y) << 0xA) >> 0xC);

	posData[1] = inst->matrix.t[1];

	posData[2] = inst->matrix.t[2] + ((MATH_Sin(instDef->rot.y) * -0x400) >> 0xC);

	return &instDef->rot.x;
}
