#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800af9f8-0x800afa60.
void AH_Door_ThDestroy(struct Thread *t)
{
	int i;
	struct WoodDoor *woodDoor = t->object;

	if (woodDoor->otherDoor != NULL)
	{
		INSTANCE_Death(woodDoor->otherDoor);
		woodDoor->otherDoor = NULL;
	}

	for (i = 0; i < 4; i++)
	{
		INSTANCE_Death(woodDoor->keyInst[i]);
		woodDoor->keyInst[i] = NULL;
	}
	return;
}
