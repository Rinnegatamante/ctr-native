#include <common.h>

struct Item *LIST_RemoveBack(struct LinkedList *L)
{
	return LIST_RemoveMember(L, L->last);
}

// ~0x80 bytes of free room here
