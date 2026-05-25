#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b5090-0x800b5210.

void RB_Fruit_GetScreenCoords(struct PushBuffer *pb, struct Instance *inst, s16 *output);

int RB_CtrLetter_ThCollide(struct Thread *letterTh, struct Thread *driverTh, void *funcThCollide, struct ScratchpadStruct *sps)
{
	s16 posScreen[2];
	struct Driver *driver;
	struct Instance *letterInst;
	struct PushBuffer *pb;

	if (sps->Input1.modelID != DYNAMIC_PLAYER)
		return 0;

	letterInst = letterTh->inst;
	driver = driverTh->object;

	pb = &sdata->gGT->pushBuffer[driver->driverID];
	RB_Fruit_GetScreenCoords(pb, letterInst, &posScreen[0]);

	driver->PickupLetterHUD.startX = pb->rect.x + posScreen[0];
	driver->PickupLetterHUD.startY = pb->rect.y + posScreen[1] - 0x14;
	driver->PickupLetterHUD.cooldown = 10;
	driver->PickupLetterHUD.numCollected++;
	driver->PickupLetterHUD.modelID = letterInst->model->id;

	letterInst->scale[0] = 0;
	letterInst->scale[1] = 0;
	letterInst->scale[2] = 0;
	letterInst->thread = 0;
	letterInst->flags |= 0x80;

	OtherFX_Play(100, 1);
	letterTh->flags |= 0x800;

	return 1;
}
