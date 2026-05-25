#include <common.h>

struct Item *LIST_RemoveFront(struct LinkedList *L)
{
	return LIST_RemoveMember(L, L->first);
}

// ~0x80 bytes of free room here
