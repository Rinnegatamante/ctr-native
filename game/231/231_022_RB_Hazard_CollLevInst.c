#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ad9ac-0x800ada90.
u16 RB_Hazard_CollLevInst(struct ScratchpadStruct *sps, struct Thread *th)
{
	u16 flag;
	s16 model;
	struct Instance *inst;
	struct InstDef *instdef;
	struct MetaDataMODEL *meta;

	// Check if the hitbox flag has the collision bit set and if InstDef is not NULL
	if ((sps->bspHitbox->flag & 0x80) && (instdef = sps->bspHitbox->data.hitbox.instDef) != NULL)
	{
		inst = instdef->ptrInstance;
		if (inst == NULL)
			return 1;

		model = inst->model->id;

		// Get the metadata for the model
		meta = COLL_LevModelMeta(model);

		// Check if LInC is not nullptr
		if ((meta != NULL) && (meta->LInC != NULL))
		{
			// Execute LInC, create a thread for this instance, and let it run thread->funcThCollide upon collision
			flag = meta->LInC(inst, th, sps);

			// if not PU_WUMPA_FRUIT
			if (model != 2)
			{
				// useless
				if (model < 2)
				{
					return flag;
				}

				// anything except for
				// 7: PU_FRUIT_CRATE,
				// 8: PU_RANDOM_CRATE (weapon box)
				if (8 < model)
				{
					return flag;
				}
				if (model < 7)
				{
					return flag;
				}
			}
			return 0;
		}
	}

	// make potion open teeth,
	// or make warpball turn around
	return 1;
}
