#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8001c9e4-0x8001ca64.
void CDSYS_SpuEnableIRQ()
{
	for (int i = 0; i < 0x200; i++)
		sdata->SpuDecodedBuf[i] = 0;

	sdata->XA_MaxSampleVal = 0;
	sdata->XA_MaxSampleValInArr = 0;

	SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
	SpuSetTransferCallback(CDSYS_SpuCallbackTransfer);
	SpuSetIRQCallback(CDSYS_SpuCallbackIRQ);

	sdata->irqAddr = 0x200;
	sdata->unused_8008d700 = 0;

	SpuSetIRQAddr(0x200);
	SpuSetIRQ(1);
}
