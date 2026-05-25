#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8001ca64-0x8001ca98.
void CDSYS_SpuDisableIRQ()
{
	SpuSetIRQ(0);
	SpuSetIRQCallback(0);
	SpuSetTransferCallback(0);

	sdata->XA_MaxSampleValInArr = 0;
	sdata->XA_MaxSampleVal = 0;
}
