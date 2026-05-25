#include <common.h>

void LIST_Clear(struct LinkedList *L)
{
	L->first = 0;
	L->last = 0;
	L->count = 0;
}
