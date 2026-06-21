#include <common.h>

static const s16 s_shieldGrowScale[8][2] = {
    {977, 1835}, {1792, 2936}, {2205, 2095}, {2335, 1254}, {1884, 1612}, {1433, 1971}, {1612, 1881}, {1792, 1792},
};

static const s16 s_shieldPulseScale[6][2] = {
    {1845, 1756}, {1899, 1720}, {1971, 1684}, {1899, 1720}, {1845, 1756}, {1792, 1792},
};

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0454-0x800b0dbc.
// NOTE(aalhendi): Native uses extracted shield scale tables from RDATA 0x800b2cf4 and 0x800b2d40.
void RB_ShieldDark_ThTick_Grow(struct Thread *th)
{
	u16 shieldFlags;
	s16 sVar4;
	int i;
	int rotY;
	struct TrackerWeapon *tw;
	struct PushBuffer *pb;

	struct GameTracker *gGT = sdata->gGT;
	struct Instance *shieldInst = th->inst;
	struct Shield *shield = th->object;
	struct Instance *colorInst = shield->instColor;
	struct Instance *highlightInst = shield->instHighlight;

	struct Thread *playerTh = th->parentThread;
	struct Driver *player = playerTh->object;
	struct Instance *driverInst = playerTh->inst;

	// if highlight cooldown is gone
	if (shield->highlightTimer == 0)
	{
		shield->highlightRot.y += 0x100;

		highlightInst->flags &= ~HIDE_MODEL;

		rotY = shield->highlightRot.y;

		int iVar8 = rotY;
		if (rotY < 0)
			iVar8 = rotY + 0xfff;

		// if highlight is finished
		if ((rotY + (iVar8 >> 12) * -0x1000) == 0x400)
		{
			// cooldown is 30 frames (one second)
			shield->highlightTimer = 30;

			shield->highlightRot.y = 0xc00;

			highlightInst->flags |= HIDE_MODEL;
		}
	}

	// if highlight cooldown is not done
	else
	{
		// decrease counter, make invisible when this is zero
		shield->highlightTimer--;

		highlightInst->flags |= HIDE_MODEL;

		// if timer runs out (last frame)
		if (shield->highlightTimer == 0)
		{
			highlightInst->flags &= ~HIDE_MODEL;
		}
	}

	struct InstDrawPerPlayer *idpp = INST_GETIDPP(shieldInst);
	struct InstDrawPerPlayer *colorIdpp = INST_GETIDPP(colorInst);
	struct InstDrawPerPlayer *highlightIdpp = INST_GETIDPP(highlightInst);

	// If the driver that used this weapon is visible
	if (player->invisibleTimer == 0)
	{
		for (i = 0; i < gGT->numPlyrCurrGame; i++)
		{
			pb = &gGT->pushBuffer[i];
			idpp[i].pushBuffer = pb;
			colorIdpp[i].pushBuffer = pb;
			highlightIdpp[i].pushBuffer = pb;
		}
	}

	// if driver is not invisible
	else
	{
		for (i = 0; i < gGT->numPlyrCurrGame; i++)
		{
			if (i == player->driverID)
				continue;

			idpp[i].pushBuffer = 0;
			colorIdpp[i].pushBuffer = 0;
			highlightIdpp[i].pushBuffer = 0;
		}
	}

	SVec3 pos;
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;

	// Copy matrix
	// To: shield instance, highlight instance, etc
	// From: thread (shield) -> parentthread (player) -> object (driver) -> instance
	LHMatrix_Parent(shieldInst, driverInst, (SVECTOR *)pos.v);
	LHMatrix_Parent(colorInst, driverInst, (SVECTOR *)pos.v);
	LHMatrix_Parent(highlightInst, driverInst, (SVECTOR *)pos.v);

	// set rotation variables
	*(int *)&shieldInst->matrix.m[0][0] = 0x1000;
	*(int *)&shieldInst->matrix.m[0][2] = 0;
	*(int *)&shieldInst->matrix.m[1][1] = 0x1000;
	*(int *)&shieldInst->matrix.m[2][0] = 0;
	shieldInst->matrix.m[2][2] = 0x1000;

	// set rotation variables
	*(int *)&colorInst->matrix.m[0][0] = 0x1000;
	*(int *)&colorInst->matrix.m[0][2] = 0;
	*(int *)&colorInst->matrix.m[1][1] = 0x1000;
	*(int *)&colorInst->matrix.m[2][0] = 0;
	colorInst->matrix.m[2][2] = 0x1000;

	// convert 3 rotation shorts into rotation matrix
	ConvertRotToMatrix(&highlightInst->matrix, &shield->highlightRot);

	s16 scaleXZ;
	s16 scaleY;

	// if animation is not done
	if (shield->animFrame < 8)
	{
		scaleXZ = s_shieldGrowScale[shield->animFrame][0];
		scaleY = s_shieldGrowScale[shield->animFrame][1];

		// set scale
		shieldInst->scale.x = scaleXZ;
		shieldInst->scale.y = scaleY;
		shieldInst->scale.z = scaleXZ;

		// set scale
		colorInst->scale.x = scaleXZ;
		colorInst->scale.y = scaleY;
		colorInst->scale.z = scaleXZ;

		// next frame
		shield->animFrame++;
	}

	// if animation is done
	else
	{
		s16 timerIndex = ((gGT->timer >> 0) % 6);

		scaleXZ = s_shieldPulseScale[timerIndex][0];
		scaleY = s_shieldPulseScale[timerIndex][1];

		// set scale
		shieldInst->scale.x = scaleXZ;
		shieldInst->scale.y = scaleY;
		shieldInst->scale.z = scaleXZ;

		// set scale
		colorInst->scale.x = scaleXZ;
		colorInst->scale.y = scaleY;
		colorInst->scale.z = scaleXZ;

		// set scale
		highlightInst->scale.x = scaleXZ;
		highlightInst->scale.y = scaleY;
		highlightInst->scale.z = scaleXZ;
	}

	// if this is not a blue shield,
	// meaning it must fade eventually
	if ((shield->flags & 4) == 0)
	{
		// duration
		s16 duration = shield->duration;

		// if out of time
		if (duration == 0)
		{
			// erase bubble instance from driver
			player->instBubbleHold = NULL;

			goto LAB_800b0d6c;
		}

		// subtract 32ms by hand
		duration -= 32;
		shield->duration = duration;

		// 2.0 seconds
		if (duration < 1920)
		{
			sVar4 = (s16)(((60 - (duration >> 5)) * 3072) / 60) + 0x400;

			// transparency
			shieldInst->alphaScale = sVar4;
			colorInst->alphaScale = sVar4;
			highlightInst->alphaScale = sVar4;
		}
	}

	shieldFlags = shield->flags;

	if (((shieldFlags & 1) != 0) || ((shieldFlags & 8) != 0) ||

	    // if race ended for this driver
	    ((player->actionsFlagSet & ACTION_RACE_FINISHED) != 0) ||

	    // if driver is being mask grabbed
	    (player->kartState == KS_MASK_GRABBED))
	{
		if ((shieldFlags & 8) != 0)
		{
			pb = &gGT->pushBuffer[player->driverID];

			pb->fadeFromBlack_currentValue = 0x1fff;
			pb->fadeFromBlack_desiredResult = 0x1000;
			pb->fade_step = -(0x88);
		}

		shield->animFrame = 0;
		player->instBubbleHold = NULL;

		// execute, then assign per-frame funcPtr to thread
		ThTick_SetAndExec(th, RB_ShieldDark_ThTick_Pop);
		return;
	}

	if ((shieldFlags & 2) == 0)
		return;

	player->instBubbleHold = NULL;
	player->numTimesMissileLaunched++;

	GAMEPAD_ShockFreq(player, 8, 0);
	GAMEPAD_ShockForce1(player, 8, 0x7f);

	// green shield
	u8 model = 0x5e;

	if ((shieldFlags & 4) != 0)
	{
		// blue shield
		model = 0x56;
	}

	// create a thread, get an instance
	struct Instance *bombInst = INSTANCE_BirthWithThread(model, 0, MEDIUM, OTHER, RB_MovingExplosive_ThTick, sizeof(struct TrackerWeapon), playerTh);

	struct Thread *bombTh = bombInst->thread;
	bombTh->funcThDestroy = PROC_DestroyInstance;

	// if driver is not an AI (human)
	if ((player->actionsFlagSet & ACTION_BOT) == 0)
	{
		// make driver talk
		Voiceline_RequestPlay(13, data.characterIDs[player->driverID], 0x10);
	}

	// copy position and rotation from one instance to another
	*(int *)&bombInst->matrix.m[0][0] = *(int *)&shieldInst->matrix.m[0][0];
	*(int *)&bombInst->matrix.m[0][2] = *(int *)&shieldInst->matrix.m[0][2];
	*(int *)&bombInst->matrix.m[1][1] = *(int *)&shieldInst->matrix.m[1][1];
	*(int *)&bombInst->matrix.m[2][0] = *(int *)&shieldInst->matrix.m[2][0];
	bombInst->matrix.m[2][2] = shieldInst->matrix.m[2][2];
	bombInst->matrix.t[0] = shieldInst->matrix.t[0];
	bombInst->matrix.t[1] = shieldInst->matrix.t[1];
	bombInst->matrix.t[2] = shieldInst->matrix.t[2];

	// set scale (x, y, z) and transparency
	bombInst->scale.x = 0x400;
	bombInst->scale.y = 0x400;
	bombInst->scale.z = 0x400;
	bombInst->alphaScale = 0x400;

	// get object from thread
	tw = bombTh->object;

	tw->flags = 0;
	tw->driverTarget = 0;
	tw->timeAlive = 0;
	tw->audioPtr = 0;
	tw->frameCount_Blind = 0;

	tw->driverParent = player;
	tw->instParent = driverInst;

	// do NOT patch for 60fps,
	// velocity uses elapsedTime
	tw->vel.y = 0;
	tw->vel.x = (driverInst->matrix.m[0][2] * 3) >> 7;
	tw->vel.z = (driverInst->matrix.m[2][2] * 3) >> 7;

	tw->rotY = player->angle;
	tw->frameCount_DontHurtParent = 10;

LAB_800b0d6c:

	// green shield fade away sound
	PlaySound3D(0x58, shieldInst);

	// shield and highlight
	INSTANCE_Death(colorInst);
	INSTANCE_Death(highlightInst);

	// This thread is now dead
	th->flags |= THREAD_FLAG_DEAD;
}
