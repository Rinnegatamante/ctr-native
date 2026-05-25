#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80018b18-0x80018ba0
int CAM_Path_GetNumPoints(void)
{
	struct GameTracker *gGT;
	struct Level *level1;
	struct SpawnType1 *ptrSpawnType1;
	s16 *introCam;
	u16 uVar4;

	uVar4 = 0;

	gGT = sdata->gGT;
	level1 = gGT->level1;
	if (level1 == NULL)
		return 0;

	ptrSpawnType1 = level1->ptrSpawnType1;
	if (ptrSpawnType1->count < 3)
		return 0;

	void **ptrs = ST1_GETPOINTERS(ptrSpawnType1);
	introCam = ptrs[ST1_CAMERA_PATH];
	if (introCam == NULL)
		return 0;

	while (1)
	{
		if (introCam[0] == 0)
			break;
		uVar4 += introCam[0];
		introCam += introCam[0] * 6 + 2;
	}

	return (s16)uVar4;
}
