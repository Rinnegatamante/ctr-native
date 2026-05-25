#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80018818-0x800188a8
void CAM_Init(struct CameraDC *cDC, int cameraID, struct Driver *d, struct PushBuffer *pb)
{
// Naughty Dog debug printf
#if BUILD == SepReview
	printf("camera init\n");
#endif

	DECOMP_PROC_BirthWithObject(0x30f, CAM_ThTick, sdata->s_camera, NULL)->inst = (struct Instance *)cDC;

	memset(cDC, 0, sizeof(struct CameraDC));

	// needed or L2 breaks
	cDC->cameraID = cameraID;

	cDC->driverToFollow = d;
	cDC->pushBuffer = pb;

	// dont set cameraMode to zero,
	// memset makes it already zero

	cDC->flags |= 8;
}
