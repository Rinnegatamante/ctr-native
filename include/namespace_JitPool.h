// complete struct
struct JitPool
{
	struct LinkedList free;
	struct LinkedList taken;

	int maxItems;
	u32 itemSize;
	int poolSize;
	void *ptrPoolData;
};

CTR_STATIC_ASSERT(sizeof(struct JitPool) == 0x28);
