#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8001c984-0x8001c9e4.
void CDSYS_SpuCallbackTransfer()
{
	if (sdata->irqAddr == 0)
		sdata->irqAddr = 0x200;
	else
		sdata->irqAddr = 0;

	SpuSetIRQAddr(sdata->irqAddr);
	SpuSetIRQ(1);

	sdata->countPass_CdTransferCallback++;

	CDSYS_SpuGetMaxSample();
}
