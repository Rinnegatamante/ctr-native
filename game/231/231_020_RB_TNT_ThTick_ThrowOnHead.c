#include <common.h>

void DECOMP_RB_TNT_ThTick_SitOnHead();

static const s16 s_tntThrowHeadY[0x10] = {32, 32, 64, 32, 32, 48, 32, 32, 48, 64, 32, 48, 56, 24, 32, 56};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ad710-0x800ad92c.
// NOTE(aalhendi): Native uses retail character-height table bytes from 0x800b2ac4.
void DECOMP_RB_TNT_ThTick_ThrowOnHead(struct Thread *t)
{
	struct MineWeapon *mw;
	struct Instance *inst;
	s16 *array;
	struct GameTracker *gGT;

	s16 rot[3];
	s16 distHead;

	// matrix?
	s16 auStack48[32];

	gGT = sdata->gGT;

	inst = t->inst;

	// object
	mw = inst->thread->object;

	// alter height of TNT as it flies onto a driver's head,
	// do NOT use parenthesis
	mw->deltaPos[1] += (mw->velocity[1] * gGT->elapsedTimeMS) >> 5;

	// if TNT is moving downward
	if (mw->velocity[1] < 0)
	{
		array = (s16 *)s_tntThrowHeadY;

		distHead = array[data.characterIDs[mw->driverTarget->driverID]];

		// if TNT landed on head
		if ((mw->deltaPos[1] < distHead) && (mw->deltaPos[1] = distHead, inst->scale[0] == 0x800))
		{
			// Set TNT timer to 0, it blows up at 0x5a
			mw->numFramesOnHead = 0;

			// number of jumps is 7 or 8
			mw->jumpsRemaining = 8 - (MixRNG_Scramble() & 1);

			// play sound that you hit a TNT
			PlaySound3D(0x51, inst);

			inst->unk50 = mw->driverTarget->instSelf->unk50 + -1;
			inst->unk51 = mw->driverTarget->instSelf->unk51 + -1;

			// assign
			ThTick_SetAndExec(t, DECOMP_RB_TNT_ThTick_SitOnHead);
			return;
		}
	}

	// CopyMatrix
	LHMatrix_Parent(inst, mw->driverTarget->instSelf, (SVECTOR *)&mw->deltaPos[0]);

	// rotation
	rot[0] = 0;
	rot[1] = mw->tntSpinY;
	rot[2] = 0;

	// convert 3 rotation shorts into rotation matrix
	ConvertRotToMatrix((MATRIX *)auStack48, &rot[0]);

	MatrixRotate(&inst->matrix, &inst->matrix, (MATRIX *)auStack48);

	// reduce time remaining until TNT lands on head
	mw->velocity[1] -= ((gGT->elapsedTimeMS << 2) >> 5);

	// set a minimum value (-0x60)
	if (mw->velocity[1] < -0x60)
		mw->velocity[1] = -0x60;

	// rotation
	mw->tntSpinY += 0x100;

	// if scale is small
	if (inst->scale[0] < 0x801)
	{
		// set min scale
		inst->scale[0] = 0x800;
		inst->scale[1] = 0x800;
		inst->scale[2] = 0x800;
	}

	// if scale is large
	else
	{
		// reduce scale
		inst->scale[0] -= 0x100;
		inst->scale[1] -= 0x100;
		inst->scale[2] -= 0x100;
	}
	return;
}
