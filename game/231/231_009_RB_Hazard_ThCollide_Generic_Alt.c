#include <common.h>

void RB_Hazard_ThCollide_Generic(struct Thread *thread);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac3f8-0x800ac42c.
void RB_Hazard_ThCollide_Generic_Alt(struct Thread **param_1)
{
	RB_Hazard_ThCollide_Generic(param_1[0]);
}
