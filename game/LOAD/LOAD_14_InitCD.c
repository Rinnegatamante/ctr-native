#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80031c58-0x80031c78 for the retail path.
void DECOMP_LOAD_InitCD()
{
#ifdef USE_PCDRV
	PCinit();
	CDSYS_Init(0);
	return;
#endif

	CDSYS_Init(1);
}
