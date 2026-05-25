#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003ee20-0x8003eeb0
void Particle_FuncPtr_ExhaustUnderwater(struct Particle *p)
{
	struct IconGroup *icon;

	if ((3 < ((p->axis[1].startVal >> 8) + p->driverInst->matrix.t[1])) && (p->framesLeftInLife < 27))
	{
		// bubblepop
		icon = sdata->gGT->iconGroup[8];
		p->ptrIconGroup = icon;

		if (icon != NULL)
		{
			struct Icon **ptrIconArray = ICONGROUP_GETICONS(icon);

			// actually the first icon pointer in the array,
			// not the pointer to the array itself
			p->ptrIconArray = ptrIconArray[0];
		}

		p->axis[4].startVal = MixRNG_Scramble() & 0xfff;
		p->framesLeftInLife = 0;
	}
}
