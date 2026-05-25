#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b5210-0x800b52dc.

int RB_CtrLetter_LInC(struct Instance *letterInst, struct Thread *driverTh, struct ScratchpadStruct *sps)
{
	typedef int (*CtrLetterCollideFunc)(struct Thread *, struct Thread *, void *, struct ScratchpadStruct *);
	struct Thread *letterTh;

	letterTh = letterInst->thread;
	if (letterTh == NULL)
	{
		letterTh = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(4, NONE, SMALL, STATIC), RB_CtrLetter_ThTick, "ctr", NULL);

		letterInst->thread = letterTh;
		if (letterTh == NULL)
			return 0;

		letterTh->inst = letterInst;
		letterTh->funcThCollide = (void (*)(struct Thread *))RB_CtrLetter_ThCollide;
		letterTh = letterInst->thread;
	}

	if ((letterTh == NULL) || (letterTh->funcThCollide == NULL))
		return 0;

	if (letterInst->scale[0] == 0)
		return 0;

	return ((CtrLetterCollideFunc)letterTh->funcThCollide)(letterTh, driverTh, letterTh->funcThCollide, sps);
}
