#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac694-0x800ac714
char CS_Instance_BoolPlaySound(struct CutsceneObj *cs, struct Instance *desiredInst)
{
	struct Instance *inst;
	struct Instance **visInstSrc;
	struct InstDrawPerPlayer *idpp;

	if ((desiredInst == NULL) || ((cs->flags & 0x1000) == 0))
		return 1;

	// pointer to array of visible instances
	visInstSrc = sdata->gGT->cameraDC[0].visInstSrc;

#if defined(CTR_NATIVE)
	// NOTE(aalhendi): Same native low-RAM guard as AH_WarpPad_ThTick:
	// a null camera list behaves like "desired instance is not visible."
	if (visInstSrc == NULL)
		return 0;
#endif

	// Same code as warppad_thtick
	while (visInstSrc[0] != 0)
	{
		if (visInstSrc[0] == desiredInst)
		{
			idpp = INST_GETIDPP(desiredInst);
			return (idpp[0].instFlags & 0x40) != 0;
		}

		visInstSrc++;
	}

	return 0;
}
