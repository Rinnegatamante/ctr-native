#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b1ef8-0x800b1f78.
void AH_Pause_Destroy(void)
{
	int i;

	// global -> register
	struct PauseObject *ptrPauseObject = D232.ptrPauseObject;

	// check register
	if (ptrPauseObject == 0)
		return;

	// loop through 14 instances, destroy them
	for (i = 0; i < 0xe; i++)
	{
		INSTANCE_Death(ptrPauseObject->PauseMember[i].inst);
	}

	// kill thread
	D232.ptrPauseObject = 0;
	ptrPauseObject->t->flags |= THREAD_FLAG_DEAD;
}
