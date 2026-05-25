#include <common.h>

// ASM: 8004205c-80042288
struct Thread *DECOMP_PROC_BirthWithObject(int flags, void *funcThTick, char *name, struct Thread *relativeTh)
{
	int bucketID;
	struct JitPool *stackPool;
	void *stackObj;
	struct Thread *th;
	struct GameTracker *gGT;

	gGT = sdata->gGT;

	// determine bucketID from relativeTh or flags
	if (relativeTh != 0)
		bucketID = relativeTh->flags & 0xff;
	else
		bucketID = flags & 0xff;

	// select stack pool based on flags & 0x300
	switch (flags & 0x300)
	{
	case 0x100: // largeStack
		stackPool = &gGT->JitPools.largeStack;
		break;
	case 0x200: // mediumStack
		stackPool = &gGT->JitPools.mediumStack;
		break;
	default: // 0x300 = smallStack
		stackPool = &gGT->JitPools.smallStack;
		break;
	}

	// allocate stack object FIRST
	stackObj = LIST_RemoveFront(&stackPool->free);

	// validate bucket
	if (bucketID >= NUM_BUCKETS)
	{
		if (stackObj != 0)
			DECOMP_PROC_DestroyObject((void *)((u32)stackObj + 8), flags);
		return 0;
	}

	// validate size fits in pool
	if ((u32)(flags >> 0x10) >= (stackPool->itemSize - 8))
	{
		if (stackObj != 0)
			DECOMP_PROC_DestroyObject((void *)((u32)stackObj + 8), flags);
		return 0;
	}

	// check stack object allocated
	if (stackObj == 0)
		return 0;

	// allocate thread SECOND
	th = (struct Thread *)LIST_RemoveFront(&gGT->JitPools.thread.free);

	// check thread allocated
	if (th == 0)
	{
		DECOMP_PROC_DestroyObject((void *)((u32)stackObj + 8), flags);
		return 0;
	}

	// initialize thread fields
	th->flags = flags;
	th->cooldownFrameCount = 0;
	th->funcThCollide = 0;
	th->funcThDestroy = 0;
	th->inst = 0;

	// handle relative thread linking
	if (relativeTh == 0)
	{
		struct ThreadBucket *tb = &gGT->threadBuckets[bucketID];

		th->siblingThread = tb->thread;
		tb->thread = th;
		th->parentThread = 0;
		th->childThread = 0;
	}
	else if (flags & SELF_SIBLING)
	{
		th->siblingThread = relativeTh->siblingThread;
		relativeTh->siblingThread = th;
		th->childThread = 0;
		th->parentThread = relativeTh->parentThread;
	}
	else if (flags & CHILD_BETWEEN)
	{
		th->childThread = relativeTh->childThread;
		relativeTh->childThread = th;
		th->parentThread = relativeTh;
		th->siblingThread = 0;
	}
	else
	{
		th->childThread = 0;
		th->siblingThread = relativeTh->childThread;
		relativeTh->childThread = th;
		th->parentThread = relativeTh;
	}

	// set remaining fields AFTER linking (ASM order)
	th->funcThTick = funcThTick;
	th->name = name;
	th->object = (void *)(((u32)stackObj) + 8);

	return th;
}
