#include <common.h>

int DECOMP_JitPool_Add(struct JitPool *AP)
{
	struct Item *item;
	item = AP->free.first;

	if (item != 0)
	{
		LIST_RemoveMember(&AP->free, item);
		LIST_AddFront(&AP->taken, item);
	}

	return (int)item;
}
