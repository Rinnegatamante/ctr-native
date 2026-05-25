#include <common.h>

void DECOMP_INSTANCE_LevDelayedLInBs(struct Instance *instDefs, u32 numInstances)
{
	struct InstDef *id = instDefs->instDef;
	for (u32 i = 0; i < numInstances; i++)
	{
		int *puVar2 = (int *)id;
		void *asdf = COLL_LevModelMeta((int)*(s16 *)(puVar2[-7] + 0x10));
		if (asdf != NULL && *(int *)(asdf + 16) != (int)NULL) // if pointer is not nullptr && if LInB
			((void (*)(struct InstDef *))(asdf + 16))(id);    // execute LInB for this instance
		id++;                                                 // next InstDef
	}
}
