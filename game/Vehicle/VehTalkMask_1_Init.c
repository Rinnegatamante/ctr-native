#include <common.h>

void DECOMP_VehTalkMask_ThTick(struct Thread *t);

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80069178-0x800691e4.
struct Instance *DECOMP_VehTalkMask_Init()
{
	sdata->boolIsMaskThreadAlive = 1;
	sdata->talkMask_boolDead = 0;

	struct Instance *mhInst = DECOMP_INSTANCE_BirthWithThread(0x39, sdata->s_head, SMALL, AKUAKU, DECOMP_VehTalkMask_ThTick, 6, 0);

	struct Thread *mhTh = mhInst->thread;
	mhTh->funcThDestroy = DECOMP_PROC_DestroyInstance;

	((struct MaskHint *)mhTh->object)->scale = 0;

	return mhInst;
}
