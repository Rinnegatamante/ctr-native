#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80042910-0x80042974.
void PushBuffer_SetPsyqGeom(struct PushBuffer *pb)
{
	gte_SetGeomOffset(pb->rect.w / 2, pb->rect.h / 2);
	gte_SetGeomScreen(pb->distanceToScreen_PREV);
	return;
}
