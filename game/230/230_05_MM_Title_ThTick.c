#include <common.h>

static void MM_Title_RotMatrixMul(MATRIX *matrix, const SVec3 *input, VECTOR *mac)
{
	gte_SetRotMatrix(matrix);
	gte_ldv0((SVECTOR *)input);
	gte_rtv0();
	gte_stlvnl(mac);
}

static void MM_Title_UpdateTrophySpecLight(struct Instance *titleInst)
{
	struct GameTracker *gGT = sdata->gGT;
	struct PushBuffer *pb = &gGT->pushBuffer[0];
	struct InstDrawPerPlayer *idpp = INST_GETIDPP(titleInst);
	MATRIX matrix;
	SVec3 rot;
	SVec3 light;
	SVec3 view;
	VECTOR lightMac;
	VECTOR viewMac;

	rot.x = -pb->rot.x;
	rot.y = -pb->rot.y;
	rot.z = -pb->rot.z;
	ConvertRotToMatrix_Transpose(&matrix, &rot);

	light.x = 0;
	light.y = 0x1000;
	light.z = 0;
	MM_Title_RotMatrixMul(&matrix, &light, &lightMac);

	titleInst->unk53 = (u8)lightMac.vx;
	titleInst->reflectionRGBA = (u32)lightMac.vz;

	view.x = titleInst->matrix.t[0] - pb->pos.x;
	view.y = titleInst->matrix.t[1] - pb->pos.y;
	view.z = titleInst->matrix.t[2] - pb->pos.z;
	MATH_VectorNormalize(&view);
	MM_Title_RotMatrixMul(&matrix, &view, &viewMac);

	idpp[0].halfVector.x = (s16)((u16)lightMac.vx + (u16)viewMac.vx);
	idpp[0].halfVector.y = (s16)((u16)lightMac.vy + (u16)viewMac.vy);
	idpp[0].halfVector.z = (s16)((u16)lightMac.vz + (u16)viewMac.vz);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 overlay 230 0x800ac350-0x800ac6dc.
void MM_Title_ThTick(struct Thread *title)
{
	s16 animFram;
	struct Instance *titleInst;
	int i;
	int timer;
	struct Title *ptrTitle;

	// frame counters
	timer = D230.timerInTitle;

	// If you press Cross, Circle, Triangle, or Square
	if ((sdata->buttonTapPerPlayer[0] & 0x40070) != 0)
	{
		// clear gamepad input (for menus)
		RECTMENU_ClearInput();

		// set frame to 1000, skip the animation
		D230.timerInTitle = 1000;
	}

	// cap at 230
	if (timer > 230)
		timer = 230;

	// play 8 sounds, one on each frame
	for (i = 0; i < 8; i++)
	{
		if (D230.titleSounds[i].frameToPlay == timer)
		{
			// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac3e8-0x800ac400 for title queued SFX.
			OtherFX_Play(D230.titleSounds[i].soundID, 1);
		}
	}

	// copy pointer to title object
	ptrTitle = (struct Title *)title->object;

	// loop 6 times
	for (i = 0; i < 6; i++)
	{
		// current instance
		titleInst = ptrTitle->i[i];

		titleInst->flags &= ~HIDE_MODEL;

		// the frame of title screen that each instance should start animation
		animFram = D230.titleInstances[i].frameIndex_startMoving;

		// set all instances to first animation
		titleInst->animIndex = 0;

		// set animation frame, based on what frame each instance should start
		titleInst->animFrame = (timer - animFram);

		// if instance has not started animation
		if (((timer - animFram) * 0x10000) < 0)
		{
			// skip the trophy instance
			if (i != 2)
			{
				titleInst->flags |= HIDE_MODEL;
			}

			// set animFrame to zero
			titleInst->animFrame = 0;
		}

		if ((D230.titleInstances[i].boolTrophy) != 0)
		{
			// if frame is anywhere in the two seconds
			// that the trophy is in the air
			if ((u32)(timer - 138) < 62)
			{
				titleInst->flags |= HIDE_MODEL;
			}

			// otherwise
			else if (200 <= timer)
			{
				// play frame index, based on total animation frame
				titleInst->animFrame = timer - 200;

				// set animation to 1
				titleInst->animIndex = 1;
			}

			MM_Title_UpdateTrophySpecLight(titleInst);
		}
	}

	MM_Title_CameraMove(ptrTitle, timer);

	// increment frame counter
	timer = D230.timerInTitle + 1;

	if (245 < D230.timerInTitle)
	{
		// animation is over
		D230.menuMainMenu.state &= ~(DISABLE_INPUT_ALLOW_FUNCPTRS);
		D230.menuMainMenu.state |= EXECUTE_FUNCPTR;

		// dont increment index
		timer = D230.timerInTitle;
	}

	// write to index
	D230.timerInTitle = timer;
}
