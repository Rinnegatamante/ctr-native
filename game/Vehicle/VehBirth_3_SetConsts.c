#include <common.h>

void VehBirth_SetConsts(struct Driver *driver)
{
	u32 metaPhysSize;
	u32 i;
	struct MetaPhys *metaPhys;
	u8 *d;

	d = (u8 *)driver;

	int engineID = data.MetaDataCharacters[data.characterIDs[driver->driverID]].engineID;

	for (i = 0; i < 65; i++)
	{
		metaPhys = &data.metaPhys[i];

		metaPhysSize = metaPhys->size;

		void *src = &metaPhys->value[engineID];
		void *dst = &d[metaPhys->offset];

		if (metaPhysSize == 1)
		{
			*(char *)dst = *(char *)src;
			continue;
		}

		if (metaPhysSize == 2)
		{
			*(s16 *)dst = *(s16 *)src;
			continue;
		}

		*(int *)dst = *(int *)src;
	}

	return;
}
