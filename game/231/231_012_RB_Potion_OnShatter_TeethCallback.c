#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac5e8-0x800ac638.
// Required to make door open when driver hits potion, or potion shatters due to full MinePool.
void RB_Potion_OnShatter_TeethCallback(int unk, struct BSP *bspHitbox)
{
	struct InstDef *instDef;
	struct Instance *teethInst;

	instDef = bspHitbox->data.hitbox.instDef;
	if (instDef != NULL)
		if (teethInst = instDef->ptrInstance, teethInst != NULL)
			if (instDef->modelID == STATIC_TEETH) // tiger temple door
				RB_Teeth_OpenDoor(teethInst);
}
