#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac6b4-0x800aca50.
void RB_Potion_ThTick_InAir(struct Thread *t)
{
	struct GameTracker *gGT;
	struct Instance *inst;
	struct MineWeapon *mw;

	SVec3 posBottom;
	SVec3 posTop;

	struct BSP *bspHitbox;
	struct InstDef *instDef;

	struct ScratchpadStruct *sps = CTR_SCRATCHPAD_PTR(struct ScratchpadStruct, 0x108);

	gGT = sdata->gGT;
	inst = t->inst;
	mw = t->object;

	// adjust position, by velocity, do NOT use parenthesis
	inst->matrix.t[0] += mw->velocity[0] * gGT->elapsedTimeMS >> 5;
	inst->matrix.t[1] += mw->velocity[1] * gGT->elapsedTimeMS >> 5;
	inst->matrix.t[2] += mw->velocity[2] * gGT->elapsedTimeMS >> 5;

	// gravity, decrease velocity over time
	mw->velocity[1] -= ((gGT->elapsedTimeMS << 2) >> 5);

	// terminal velocity
	if (mw->velocity[1] < -0x60)
		mw->velocity[1] = -0x60;

	mw->cooldown -= gGT->elapsedTimeMS;

	if (mw->cooldown < 0)
		mw->cooldown = 0;

	posBottom.x = inst->matrix.t[0];
	posBottom.y = inst->matrix.t[1] - 0x40;
	posBottom.z = inst->matrix.t[2];

	posTop.x = inst->matrix.t[0];
	posTop.y = inst->matrix.t[1] + 0x100;
	posTop.z = inst->matrix.t[2];

	sps->Union.QuadBlockColl.searchFlags = COLL_SEARCH_TEST_INSTANCES | COLL_SEARCH_FORCE_INSTANCE_HIT;
	sps->Union.QuadBlockColl.quadFlagsWanted = QUADBLOCK_FLAG_GROUND | QUADBLOCK_FLAG_TRIGGER;
	sps->Union.QuadBlockColl.quadFlagsIgnored = 0;

	if (gGT->numPlyrCurrGame < 3)
	{
		sps->Union.QuadBlockColl.searchFlags = COLL_SEARCH_TEST_INSTANCES | COLL_SEARCH_HIGH_LOD | COLL_SEARCH_FORCE_INSTANCE_HIT;
	}

	sps->ptr_mesh_info = gGT->level1->ptr_mesh_info;

	COLL_SearchBSP_CallbackQUADBLK(&posBottom, &posTop, sps, 0);

	RB_MakeInstanceReflective(sps, inst);

	if ((sps->collision.stepFlags & COLL_STEP_TRIGGER_WEAPON_REACT) != 0)
	{
		RB_GenericMine_ThDestroy(t, inst, mw);
	}

	int iVar4;
	int iVar5;

	// did not hit BSP hitbox
	if (sps->boolDidTouchHitbox == 0)
	{
		if (sps->boolDidTouchQuadblock != 0)
		{
			VehPhysForce_RotAxisAngle(&inst->matrix, sps->hit.plane.normal.v, 0);

			iVar4 = sps->Union.QuadBlockColl.hitPos.y;
			iVar5 = inst->matrix.t[1];

			if (iVar4 + 0x30 < iVar5)
				return;

			// if no cooldown
			if (mw->cooldown == 0)
			{
				// set position to where quadblock was hit
				inst->matrix.t[1] = iVar4;

				mw->stopFallAtY = iVar4;
				mw->cooldown = 0xf00; // 3.84s
				mw->velocity[0] = 0;
				mw->velocity[1] = 0;
				mw->velocity[2] = 0;
				mw->extraFlags &= 0xfffd; // remove "thrown" flag

				ThTick_SetAndExec(t, RB_GenericMine_ThTick);
				return;
			}

			// if instance is under hitPos, move up
			if (iVar5 <= iVar4)
				inst->matrix.t[1] = iVar4;

			// if distance to move back to quadblock < velocity
			if (mw->velocity[1] < (inst->matrix.t[1] - iVar5) + 0x28)
				mw->velocity[1] = (inst->matrix.t[1] - iVar5) + 0x28;

			return;
		}

		// if did not touch quadblock in range [-0x40, 0x100],
		// check again with range [-0x900, 0x100]

		// posBottom
		posBottom.x = inst->matrix.t[0];
		posBottom.y = inst->matrix.t[1] - 0x900;
		posBottom.z = inst->matrix.t[2];

		COLL_SearchBSP_CallbackQUADBLK(&posBottom, &posTop, sps, 0);

		// quadblock exists far below potion, dont destroy
		if (sps->boolDidTouchQuadblock != 0)
			return;
	}

	// hit BSP hitbox, and instance is TEETH
	else
	{
		bspHitbox = sps->bspHitbox;

		if ((
		        // bsp->flags & hitbox
		        ((bspHitbox->flag & 0x80) != 0) && (
		                                               // hitbox contains instDef
		                                               instDef = bspHitbox->data.hitbox.instDef, instDef != 0)) &&

		    // instDef->modelID == TEETH
		    (instDef->modelID == STATIC_TEETH) &&

		    // instDef->instance exists
		    (instDef->ptrInstance != 0))
		{
			// if door is open, quit
			if ((sdata->doorAccessFlags & 1) == 1)
				return;

			// open door if door is closed,
			// then destroy mine right after
			RB_Teeth_OpenDoor(instDef->ptrInstance);
		}
	}

	// hit TEETH door,
	// or no quadblock exists within 0x900 units of Y axis
	RB_GenericMine_ThDestroy(t, inst, mw);

	return;
}
