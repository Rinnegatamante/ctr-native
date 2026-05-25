#include <common.h>

static s32 CollMoved_ScrubImpact_Abs(s32 value)
{
	return (value < 0) ? (s32)(0u - (u32)value) : value;
}

static u32 CollMoved_ScrubImpact_PackXY(s32 x, s32 y)
{
	return ((u32)(u16)x) | ((u32)y << 16);
}

static s32 CollMoved_ScrubImpact_TrigX(s32 angle)
{
	struct TrigTable trig = data.trigApprox[angle & 0x3ff];
	s32 value = ((angle & 0x400) == 0) ? trig.sin : trig.cos;

	if ((angle & 0x800) != 0)
	{
		value = -value;
	}

	return value;
}

static void CollMoved_ScrubImpact_GteLLV0(s32 x, s32 y, s32 z, s32 *outX, s32 *outY, s32 *outZ)
{
	MTC2(CollMoved_ScrubImpact_PackXY(x, y), 0);
	MTC2(z, 1);
	doCOP2(0x04a6012);
	*outX = MFC2_S(25);
	*outY = MFC2_S(26);
	*outZ = MFC2_S(27);
}

static void CollMoved_ScrubImpact_GteOP12(void)
{
	doCOP2(0x0178000c);
}

static void CollMoved_ScrubImpact_ProjectWallVelocity(s16 normalX, s16 normalY, s16 normalZ, s32 oldVelX, s32 oldVelZ, s32 *outX, s32 *outY, s32 *outZ)
{
	u32 r11;
	u32 r22;
	u32 r33;
	s32 crossX;
	s32 crossY;
	s32 crossZ;

	MTC2(oldVelX, 9);
	MTC2(0, 10);
	MTC2(oldVelZ, 11);
	CTC2(normalX, 0);
	CTC2(normalY, 2);
	CTC2(normalZ, 4);

	CollMoved_ScrubImpact_GteOP12();

	r11 = CFC2(0);
	r22 = CFC2(2);
	r33 = CFC2(4);

	MTC2(r11, 9);
	MTC2(r22, 10);
	MTC2(r33, 11);

	crossX = MFC2_S(25);
	crossY = MFC2_S(26);
	crossZ = MFC2_S(27);

	CTC2(crossX, 0);
	CTC2(crossY, 2);
	CTC2(crossZ, 4);

	CollMoved_ScrubImpact_GteOP12();

	*outX = MFC2_S(25);
	*outY = MFC2_S(26);
	*outZ = MFC2_S(27);
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80020c58-0x80021500; scrub depth is SPS+0x0e.
u32 COLL_MOVED_ScrubImpact(struct Driver *d, struct Thread *t, struct ScratchpadStruct *sps, struct Scrub *scrub, int *velocity)
{
	s16 normalX = sps->Set2.normalVec[0];
	s16 normalY = sps->Set2.normalVec[1];
	s16 normalZ = sps->Set2.normalVec[2];
	s32 dot;
	u32 ret;

	if ((d->unknownTraction != 0) && (sps->boolDidTouchQuadblock != 0) && ((sps->Set2.ptrQuadblock->quadFlags & 0x1000) != 0) && (((u8 *)sps)[0x7e] != 6) &&
	    (sps->Set2.ptrQuadblock != d->underDriver))
	{
		if ((CollMoved_ScrubImpact_Abs(d->speedApprox) < 0x300) && (CollMoved_ScrubImpact_Abs(d->jumpHeightCurr) < 0x300) && (d->fireSpeed == 0))
		{
			s32 diffX = (d->posCurr.x >> 8) - sps->Set2.hitPos[0];
			s32 diffZ = (d->posCurr.z >> 8) - sps->Set2.hitPos[2];
			s32 diffY = (d->posCurr.y >> 8) - sps->Set2.hitPos[1];

			if ((diffX | diffY | diffZ) != 0)
			{
				s32 len = VehCalc_FastSqrt((u32)CollFixed_MulLo(diffX, diffX) + (u32)CollFixed_MulLo(diffY, diffY) + (u32)CollFixed_MulLo(diffZ, diffZ), 0);

				normalX = CollFixed_Sll32(diffX, 12) / len;
				normalY = CollFixed_Sll32(diffY, 12) / len;
				normalZ = CollFixed_Sll32(diffZ, 12) / len;
			}
		}
	}

	dot = ((velocity[0] >> 3) * normalX + (velocity[1] >> 3) * normalY + (velocity[2] >> 3) * normalZ) >> 9;

	if (dot < -0xa00)
	{
		d->actionsFlagSet |= 0x80;
	}

	dot -= CollFixed_ReadS16(sps, 0xe);

	ret = 0;
	if (dot < 0)
	{
		u32 scrubFlags = scrub->flags;
		s32 scrubSpeed;

		if ((scrubFlags & 4) == 0)
		{
			d->actionsFlagSet |= 0x2000;
		}

		if ((scrubFlags & 8) == 0)
		{
			d->reserves = 0;
			d->turbo_outsideTimer = 0;
		}

		scrubSpeed = scrub->unk_0x8;

		if (!((d->set_0xF0_OnWallRub == 0) ? (0x3e7ff < scrubSpeed) : (d->scrubMeta8 < scrubSpeed)))
		{
			d->set_0xF0_OnWallRub = 0xf0;
			d->scrubMeta8 = scrubSpeed;
			d->posWallColl[0] = sps->Set2.hitPos[0];
			d->posWallColl[1] = sps->Set2.hitPos[1];
			d->posWallColl[2] = sps->Set2.hitPos[2];
		}

		ret = 0;

		if ((scrubFlags & 1) != 0)
		{
			s32 impactX = CollFixed_MulLo(dot, normalX) >> 12;
			s32 impactY = CollFixed_MulLo(dot, normalY) >> 12;
			s32 impactZ = CollFixed_MulLo(dot, normalZ) >> 12;
			s32 speedSq = 0;
			s32 oldVelX;
			s32 oldVelZ;
			s32 wallVelX;
			s32 wallVelY;
			s32 wallVelZ;

			if (scrub->unk_angle != 0)
			{
				speedSq = ((u32)CollFixed_MulLo(velocity[0], velocity[0]) + (u32)CollFixed_MulLo(velocity[1], velocity[1]) +
				           (u32)CollFixed_MulLo(velocity[2], velocity[2])) >>
				          15;
			}

			oldVelX = velocity[0];
			oldVelZ = velocity[2];
			velocity[0] = oldVelX - impactX;
			velocity[2] = oldVelZ - impactZ;
			velocity[1] -= impactY;

			CollMoved_ScrubImpact_GteLLV0(impactX, impactY, impactZ, &impactX, &impactY, &impactZ);

			if ((sps->boolDidTouchQuadblock != 0) && ((sps->Union.QuadBlockColl.searchFlags & 0x10) == 0) && ((d->actionsFlagSetPrevFrame & 1) == 0) &&
			    ((sps->Set2.ptrQuadblock->quadFlags & 0x1000) != 0))
			{
				CollMoved_ScrubImpact_ProjectWallVelocity(normalX, normalY, normalZ, oldVelX, oldVelZ, &wallVelX, &wallVelY, &wallVelZ);

				{
					u32 wallSpeed = VehCalc_FastSqrt((u32)CollFixed_MulLo(wallVelX, wallVelX) + (u32)CollFixed_MulLo(wallVelY, wallVelY) +
					                                     (u32)CollFixed_MulLo(wallVelZ, wallVelZ),
					                                 0x10) >>
					                8;
					s32 speedApprox = d->speedApprox;

					if ((wallSpeed != 0) && (speedApprox > 0))
					{
						sps->Union.QuadBlockColl.searchFlags |= 0x10;
						velocity[0] = CollFixed_MulLo(wallVelX, speedApprox) / (s32)wallSpeed;
						velocity[1] = CollFixed_MulLo(wallVelY, speedApprox) / (s32)wallSpeed;
						velocity[2] = CollFixed_MulLo(wallVelZ, speedApprox) / (s32)wallSpeed;
						velocity[0] -= normalX >> 1;
						velocity[1] -= normalY >> 1;
						velocity[2] -= normalZ >> 1;
					}
				}
			}

			if (((scrubFlags & 2) != 0) && (dot < -0x13ff) && ((u32)CollFixed_MulLo(impactY, impactY) + (u32)CollFixed_MulLo(impactZ, impactZ) > 0x1900000))
			{
				if (d->kartState != KS_MASK_GRABBED)
				{
					GAMEPAD_JogCon1(d, (d->simpTurnState < 1) ? 0x1f : 0x2f, 0x60);
				}

				if (scrub->unk_angle != 0)
				{
					s32 trig = CollMoved_ScrubImpact_TrigX(scrub->unk_angle);

					if ((CollFixed_MulLo((CollFixed_MulLo(speedSq, trig) >> 12), trig) >> 12) >= (CollFixed_MulLo(dot, dot) >> 15))
						return 1;
				}

				if ((d->kartState != KS_MASK_GRABBED) && ((u32)CollFixed_MulLo(impactY, impactY) + (u32)CollFixed_MulLo(impactZ, impactZ) > 0x1900000))
				{
					u32 soundFlags = 0xff8080;

					if ((d->actionsFlagSet & 0x10000) != 0)
					{
						soundFlags = 0x1ff8080;
					}

					DECOMP_OtherFX_Play_LowLevel(6, 1, soundFlags);
					Voiceline_RequestPlay(6, data.characterIDs[d->driverID], 0x10);
					GAMEPAD_ShockFreq(d, 8, 0);
					GAMEPAD_ShockForce1(d, 8, 0x7f);

					if (d->kartState == KS_DRIFTING)
					{
						s16 turnAngle = d->turnAngleCurr;

						d->turnAngleCurr = 0;
						d->angle += turnAngle;
						d->rotCurr.w -= turnAngle;
					}

					d->instSelf->animIndex = 2;
					d->instSelf->animFrame = 0;
					d->matrixArray = 4;
					d->matrixIndex = 0;

					DECOMP_VehPhysProc_SlamWall_Init(t, d);
					return 2;
				}
			}

			ret = 1;
		}
	}

	return ret;
}
