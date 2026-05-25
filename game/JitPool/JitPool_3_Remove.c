#include <common.h>

void DECOMP_JitPool_Remove(struct JitPool *AP, struct Item *item)
{
#ifdef CTR_INTERNAL
	if (AP->taken.count == 0 || AP->taken.first == 0)
	{
		fprintf(stderr, "JitPool_Remove on empty taken list! free=%d taken=%d max=%d item=%p\n", AP->free.count, AP->taken.count, AP->maxItems, (void *)item);
	}
#endif
	LIST_RemoveMember(&AP->taken, item);
	LIST_AddFront(&AP->free, item);
}
