#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af3a4-0x800af3e4.
void AH_SaveObj_ThDestroy(struct Thread *t)
{
	struct SaveObj *save;
	save = t->object;

	if (save->inst != NULL)
	{
		INSTANCE_Death(save->inst);
		save->inst = NULL;
	}
	return;
}
