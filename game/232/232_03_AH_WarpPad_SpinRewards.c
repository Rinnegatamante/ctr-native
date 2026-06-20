#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abdfc-0x800abf48.
void AH_WarpPad_SpinRewards(struct Instance *prizeInst, struct WarpPad *warppadObj, int index, int x, int y, int z)
{
	SVec3 *lightDir;
	u32 modelID;
	u32 trig;
	u32 thirds;

	ConvertRotToMatrix(&prizeInst->matrix, &warppadObj->spinRot_Prize.x);

	modelID = prizeInst->model->id;

	if (modelID != STATIC_TROPHY) // if not trophy (no lightDir on trophy)
	{
		if (modelID == STATIC_GEM) // gem
			lightDir = &warppadObj->lightDirGem;
		else
		{
			if (modelID == STATIC_RELIC) // relic
				lightDir = &warppadObj->lightDirRelic;
			else
			{
				if (modelID == STATIC_TOKEN) // token
					lightDir = &warppadObj->lightDirToken;
				else
					goto SpinReward;
			}
		}
		Vector_SpecLightSpin3D(prizeInst, &warppadObj->spinRot_Prize.x, lightDir);
	}

SpinReward:

	// initialized as 0x555*index, but not const
	thirds = warppadObj->thirds[index];

	trig = MATH_Sin(thirds);
	prizeInst->matrix.t[1] = y + ((trig << 6) >> 0xc) + 0x100;

	// do not use original "thirds",
	// set new value without "+="
	thirds = 0x555 * index + warppadObj->spinRot_Rewards.y;

	trig = MATH_Sin(thirds);
	prizeInst->matrix.t[0] = x + (trig * 0xA0 >> 0xc);

	trig = MATH_Cos(thirds);
	prizeInst->matrix.t[2] = z + (trig * 0xA0 >> 0xc);
}
