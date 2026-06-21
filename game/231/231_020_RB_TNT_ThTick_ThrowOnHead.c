#include <common.h>

static const s16 s_tntThrowHeadY[0x10] = {32, 32, 64, 32, 32, 48, 32, 32, 48, 64, 32, 48, 56, 24, 32, 56};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ad710-0x800ad92c.
// NOTE(aalhendi): Native uses retail character-height table bytes from 0x800b2ac4.
void RB_TNT_ThTick_ThrowOnHead(struct Thread *t)
{
	struct MineWeapon *mw;
	struct Instance *inst;
	s16 *array;
	struct GameTracker *gGT;

	SVec3 rot;
	s16 distHead;

	// matrix?
	s16 auStack48[32];

	gGT = sdata->gGT;

	inst = t->inst;

	// object
	mw = inst->thread->object;

	// alter height of TNT as it flies onto a driver's head,
	// do NOT use parenthesis
	mw->deltaPos.y += (mw->velocity.y * gGT->elapsedTimeMS) >> 5;

	// if TNT is moving downward
	if (mw->velocity.y < 0)
	{
		array = (s16 *)s_tntThrowHeadY;

		distHead = array[data.characterIDs[mw->driverTarget->driverID]];

		// if TNT landed on head
		if ((mw->deltaPos.y < distHead) && (mw->deltaPos.y = distHead, inst->scale.x == 0x800))
		{
			// Set TNT timer to 0, it blows up at 0x5a
			mw->numFramesOnHead = 0;

			// number of jumps is 7 or 8
			mw->jumpsRemaining = 8 - (MixRNG_Scramble() & 1);

			// play sound that you hit a TNT
			PlaySound3D(0x51, inst);

			inst->depthBiasNormal = mw->driverTarget->instSelf->depthBiasNormal + -1;
			inst->depthBiasSecondary = mw->driverTarget->instSelf->depthBiasSecondary + -1;

			// assign
			ThTick_SetAndExec(t, RB_TNT_ThTick_SitOnHead);
			return;
		}
	}

	// CopyMatrix
	LHMatrix_Parent(inst, mw->driverTarget->instSelf, (SVECTOR *)&mw->deltaPos.x);

	// rotation
	rot.x = 0;
	rot.y = mw->tntSpinY;
	rot.z = 0;

	// convert 3 rotation shorts into rotation matrix
	ConvertRotToMatrix((MATRIX *)auStack48, &rot);

	MatrixRotate(&inst->matrix, &inst->matrix, (MATRIX *)auStack48);

	// reduce time remaining until TNT lands on head
	mw->velocity.y -= ((gGT->elapsedTimeMS << 2) >> 5);

	// set a minimum value (-0x60)
	if (mw->velocity.y < -0x60)
		mw->velocity.y = -0x60;

	// rotation
	mw->tntSpinY += 0x100;

	// if scale is small
	if (inst->scale.x < 0x801)
	{
		// set min scale
		inst->scale.x = 0x800;
		inst->scale.y = 0x800;
		inst->scale.z = 0x800;
	}

	// if scale is large
	else
	{
		// reduce scale
		inst->scale.x -= 0x100;
		inst->scale.y -= 0x100;
		inst->scale.z -= 0x100;
	}
	return;
}
