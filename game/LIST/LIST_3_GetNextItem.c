#include <common.h>

void *LIST_GetNextItem(struct Item *I)
{
	return I->next;
}
