#include <common.h>

static const s16 s_shieldPopScale[11][2] = {
    {2150, 1612}, {2419, 1433}, {2508, 1344}, {2329, 1523}, {1792, 1792}, {1254, 2150}, {896, 2419}, {716, 2508}, {537, 2150}, {358, 1254}, {179, 537},
};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0278-0x800b0454.
// NOTE(aalhendi): Native uses the extracted shield-pop scale table from RDATA 0x800b2d14.
void RB_ShieldDark_ThTick_Pop(struct Thread *t)
{
	struct Shield *sh;
	struct Instance *instDark;
	struct Instance *instColor;
	struct Driver *driverOwner;
	s16 rot[3];

	sh = t->object;
	instDark = t->inst;
	instColor = sh->instColor;
	driverOwner = t->parentThread->object;

	rot[0] = 0;
	rot[1] = 0;
	rot[2] = 0;
	LHMatrix_Parent(instDark, driverOwner->instSelf, (SVECTOR *)&rot[0]);
	LHMatrix_Parent(instColor, driverOwner->instSelf, (SVECTOR *)&rot[0]);

	// set rotation
	*(int *)&instDark->matrix.m[0][0] = 0x1000;
	*(int *)&instDark->matrix.m[0][2] = 0;
	*(int *)&instDark->matrix.m[1][1] = 0x1000;
	*(int *)&instDark->matrix.m[2][0] = 0;
	instDark->matrix.m[2][2] = 0x1000;

	// set rotation
	*(int *)&instColor->matrix.m[0][0] = 0x1000;
	*(int *)&instColor->matrix.m[0][2] = 0;
	*(int *)&instColor->matrix.m[1][1] = 0x1000;
	*(int *)&instColor->matrix.m[2][0] = 0;
	instColor->matrix.m[2][2] = 0x1000;

	int animFrame = sh->animFrame;

	if (animFrame < 0xb)
	{
		// set scale
		instDark->scale[0] = s_shieldPopScale[animFrame][0];
		instDark->scale[1] = s_shieldPopScale[animFrame][1];
		instDark->scale[2] = s_shieldPopScale[animFrame][0];

		// set scale
		instColor->scale[0] = s_shieldPopScale[animFrame][0];
		instColor->scale[1] = s_shieldPopScale[animFrame][1];
		instColor->scale[2] = s_shieldPopScale[animFrame][0];

		// next frame
		sh->animFrame += 1;

		return;
	}

	// === Animation Done ===

	// play 3D sound for "shield pop"
	PlaySound3D(0x58, instDark);

	INSTANCE_Death(instColor);
	INSTANCE_Death(sh->instHighlight);

	// this thread is now dead
	t->flags |= THREAD_FLAG_DEAD;

	return;
}
