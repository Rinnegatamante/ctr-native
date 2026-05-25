#include <common.h>

struct CollFixedPlayerTrig
{
	s32 x;
	s32 z;
};

static int COLL_FIXED_PlayerSearch_Abs(int value)
{
	return (value < 0) ? (s32)(0u - (u32)value) : value;
}

static s32 COLL_FIXED_PlayerSearch_ClampByte(s32 value)
{
	if (value < 0)
		return 0;

	if (value > 0xff)
		return 0xff;

	return value;
}

static struct CollFixedPlayerTrig COLL_FIXED_PlayerSearch_Trig(s32 angle)
{
	struct TrigTable trig = data.trigApprox[angle & 0x3ff];
	struct CollFixedPlayerTrig out;

	if ((angle & 0x400) == 0)
	{
		out.x = trig.sin;
		out.z = trig.cos;

		if ((angle & 0x800) != 0)
		{
			out.x = -out.x;
			out.z = -out.z;
		}
	}
	else
	{
		out.x = trig.cos;
		out.z = -trig.sin;

		if ((angle & 0x800) != 0)
		{
			out.x = -out.x;
			out.z = -out.z;
		}
	}

	return out;
}

static u32 COLL_FIXED_PlayerSearch_CompressNormal(s16 normalX, s16 normalY, s16 normalZ, u8 driverID)
{
	return (((u16)normalX >> 6) & 0xff) | ((((u16)normalY >> 6) & 0xff) << 8) | ((((u16)normalZ >> 6) & 0xff) << 16) | ((driverID + 1) << 24);
}

static void COLL_FIXED_PlayerSearch_SetupSearch(struct ScratchpadStruct *sps, struct Driver *d)
{
	struct GameTracker *gGT = sdata->gGT;
	s16 topX = d->posCurr.x >> 8;
	s16 topY = (d->posCurr.y >> 8) + 0x80;
	s16 topZ = d->posCurr.z >> 8;
	s16 bottomY = (d->posCurr.y >> 8) - 0x100;

	d->actionsFlagSet &= 0xfffeffff;

	sps->Union.QuadBlockColl.pos[0] = topX;
	sps->Union.QuadBlockColl.pos[1] = topY;
	sps->Union.QuadBlockColl.pos[2] = topZ;

	sps->Input1.pos[0] = topX;
	sps->Input1.pos[1] = bottomY;
	sps->Input1.pos[2] = topZ;

	sps->ptr_mesh_info = gGT->level1->ptr_mesh_info;
	sps->Union.QuadBlockColl.qbFlagsIgnored = 0x10;
	sps->Union.QuadBlockColl.qbFlagsWanted = 0x3000;

	sps->Union.QuadBlockColl.searchFlags = 0;
	if (gGT->numPlyrCurrGame < 3)
	{
		sps->Union.QuadBlockColl.searchFlags = 2;
	}

	sps->boolDidTouchQuadblock = 0;
	sps->boolDidTouchHitbox = 0;
	sps->unk3C = 0;

	sps->bbox.min[0] = topX;
	sps->bbox.max[0] = topX;
	sps->bbox.min[1] = (bottomY < topY) ? bottomY : topY;
	sps->bbox.max[1] = (topY < bottomY) ? bottomY : topY;
	sps->bbox.min[2] = topZ;
	sps->bbox.max[2] = topZ;

	sps->Union.QuadBlockColl.hitPos[0] = sps->Input1.pos[0];
	sps->Union.QuadBlockColl.hitPos[1] = sps->Input1.pos[1];
	sps->Union.QuadBlockColl.hitPos[2] = sps->Input1.pos[2];
}

static void COLL_FIXED_PlayerSearch_UpdateLighting(struct ScratchpadStruct *sps, struct Driver *d, struct Instance *inst)
{
	struct LevVertex *v0 = sps->levVertHit[0];
	struct LevVertex *v1 = sps->levVertHit[1];
	struct LevVertex *v2 = sps->levVertHit[2];

	if ((v0 == NULL) || (v1 == NULL) || (v2 == NULL))
		return;

	s32 baryA = sps->barycentrics[0];
	s32 baryB = sps->barycentrics[1];
	s32 r0 = v0->color_hi[0];
	s32 g0 = v0->color_hi[1];
	s32 b0 = v0->color_hi[2];
	s32 r = (CollFixed_MulLo(baryA, v1->color_hi[0] - r0) >> 12) + (CollFixed_MulLo(baryB, v2->color_hi[0] - r0) >> 12) + r0;
	s32 g = (CollFixed_MulLo(baryA, v1->color_hi[1] - g0) >> 12) + (CollFixed_MulLo(baryB, v2->color_hi[1] - g0) >> 12) + g0;
	s32 b = (CollFixed_MulLo(baryA, v1->color_hi[2] - b0) >> 12) + (CollFixed_MulLo(baryB, v2->color_hi[2] - b0) >> 12) + b0;

	r = COLL_FIXED_PlayerSearch_ClampByte(r);
	g = COLL_FIXED_PlayerSearch_ClampByte(g);
	b = COLL_FIXED_PlayerSearch_ClampByte(b);

	s32 light = CollFixed_MulLo(((CollFixed_MulLo(r, 0x4c) >> 8) + (CollFixed_MulLo(g, 0x96) >> 8) + (CollFixed_MulLo(b, 0x1e) >> 8)), -0x20) + 0xc00;
	s32 scaledLight;

	if (light < 0)
	{
		light = 0;
	}

	scaledLight = light << 3;

	if (light > 0x1000)
	{
		light = 0x1000;
		scaledLight = 0x8000;
	}

	light = CollFixed_MulLo(scaledLight - light, 8);

	d->alphaScaleBackup = (CollFixed_MulLo(d->alphaScaleBackup, 200) + light) >> 8;
	inst->alphaScale = (CollFixed_MulLo(inst->alphaScale, 200) + light) >> 8;
}

static void COLL_FIXED_PlayerSearch_NormalizeAxis3(struct ScratchpadStruct *sps, struct Driver *d)
{
	s32 x = CollFixed_MulLo(d->AxisAngle3_normalVec[0], 5) + CollFixed_MulLo(sps->Set2.normalVec[0], 3);
	s32 y = CollFixed_MulLo(d->AxisAngle3_normalVec[1], 5) + CollFixed_MulLo(sps->Set2.normalVec[1], 3);
	s32 z = CollFixed_MulLo(d->AxisAngle3_normalVec[2], 5) + CollFixed_MulLo(sps->Set2.normalVec[2], 3);
	u32 sum = (u32)CollFixed_MulLo(x, x) + (u32)CollFixed_MulLo(y, y) + (u32)CollFixed_MulLo(z, z);
	u32 len = VehCalc_FastSqrt(sum, 0x18) >> 12;

	d->AxisAngle3_normalVec[0] = CollFixed_Sll32(x, 12) / (s32)len;
	d->AxisAngle3_normalVec[1] = CollFixed_Sll32(y, 12) / (s32)len;
	d->AxisAngle3_normalVec[2] = CollFixed_Sll32(z, 12) / (s32)len;
}

static void COLL_FIXED_PlayerSearch_NormalizeAxis2(struct Driver *d, s32 x, s32 y, s32 z)
{
	u32 sum = (u32)CollFixed_MulLo(x, x) + (u32)CollFixed_MulLo(y, y) + (u32)CollFixed_MulLo(z, z);
	u32 len = VehCalc_FastSqrt(sum, 0x18) >> 12;

	d->AxisAngle2_normalVec[0] = CollFixed_Sll32(x, 12) / (s32)len;
	d->AxisAngle2_normalVec[1] = CollFixed_Sll32(y, 12) / (s32)len;
	d->AxisAngle2_normalVec[2] = CollFixed_Sll32(z, 12) / (s32)len;
}

static int COLL_FIXED_PlayerSearch_CheckMaskGrabProgress(struct Driver *d, struct QuadBlock *quad)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Level *level = gGT->level1;

	if ((quad->quadFlags & 0x200) != 0)
		return 1;

	if ((d->kartState == KS_MASK_GRABBED) || ((d->unkAA & 1) != 0) || ((quad->quadFlags & 0x1000) == 0))
		return 0;

	if (quad->checkpointIndex == -1)
	{
		if ((u32)(gGT->levelID - GEM_STONE_VALLEY) < 5)
		{
			d->lastValid = quad;
		}
		return 0;
	}

	struct CheckpointNode *node = &level->ptr_restart_points[(u8)quad->checkpointIndex];

	if (((d->actionsFlagSet & 0x1000000) == 0) && (node->nextIndex_forward > 1) &&
	    ((((level->ptr_restart_points[0].distToFinish >> 2) << 3) < (int)(d->distanceToFinish_checkpoint - CollFixed_MulLo(node->distToFinish, 8)))))
	{
		return 1;
	}

	u16 trackLength = level->ptr_restart_points[0].distToFinish;

	if ((node->distToFinish < (CollFixed_MulLo(trackLength, 0xf) >> 4)) && (d->lastValid->checkpointIndex != -1) &&
	    ((level->ptr_restart_points[(u8)d->lastValid->checkpointIndex].distToFinish + (trackLength >> 2)) < node->distToFinish))
	{
		return 1;
	}

	d->lastValid = d->currBlockTouching;
	return 0;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8001d944-0x8001eb0c
void COLL_FIXED_PlayerSearch(struct Thread *t, struct Driver *d)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Level *level = gGT->level1;
	struct ScratchpadStruct *sps = CTR_SCRATCHPAD_PTR(struct ScratchpadStruct, 0x108);
	struct Instance *inst;
	struct QuadBlock *quad;
	s32 landingDelta;
	s32 lerpFrames;

	COLL_FIXED_PlayerSearch_SetupSearch(sps, d);

	if (d->underDriver != NULL)
	{
		COLL_FIXED_QUADBLK_TestTriangles(d->underDriver, sps);
	}

	if ((sps->boolDidTouchQuadblock == 0) && (sps->ptr_mesh_info != NULL) && (sps->ptr_mesh_info->bspRoot != NULL))
	{
		COLL_SearchBSP_CallbackPARAM(sps->ptr_mesh_info->bspRoot, &sps->bbox, COLL_FIXED_BSPLEAF_TestQuadblocks, sps);
	}

	inst = t->inst;

	if (sps->boolDidTouchQuadblock == 0)
	{
		inst->bitCompressed_NormalVector_AndDriverIndex = ((d->driverID + 1) << 24) | 0x4000;
		inst->flags &= ~REFLECTIVE;
		d->quadBlockHeight = d->posCurr.y - 0x10000;
	}
	else
	{
		quad = sps->Set2.ptrQuadblock;
		inst->bitCompressed_NormalVector_AndDriverIndex =
		    COLL_FIXED_PlayerSearch_CompressNormal(sps->Set2.normalVec[0], sps->Set2.normalVec[1], sps->Set2.normalVec[2], d->driverID);
		d->quadBlockHeight = sps->Union.QuadBlockColl.hitPos[1] << 8;
		d->unkAA |= 4;

		if ((quad->terrain_type == TERRAIN_MUD) || (quad->terrain_type == TERRAIN_WATER) || (quad->terrain_type == TERRAIN_FASTWATER))
		{
			inst->vertSplit = 0;
			inst->flags |= SPLIT_LINE;
		}

		if (gGT->numPlyrCurrGame < 2)
		{
			u16 quadFlags = quad->quadFlags;

			if ((quadFlags & 0x2000) == 0)
			{
				if ((quadFlags & 1) != 0)
				{
					inst->flags |= REFLECTIVE;
					inst->vertSplit = *(s16 *)((u8 *)level + 0x186);
				}
				else if ((quadFlags & 4) != 0)
				{
					inst->flags |= REFLECTIVE;
					inst->vertSplit = *(s16 *)((u8 *)level + 0x184);
				}
				else
				{
					inst->flags &= ~REFLECTIVE;
				}
			}
		}

		COLL_FIXED_PlayerSearch_NormalizeAxis3(sps, d);

		if ((quad->quadFlags & 0x80) != 0)
		{
			d->actionsFlagSet |= 0x10000;
		}

		d->underDriver = quad;

		if ((d->posCurr.y <= d->quadBlockHeight + 0x1000) || ((quad->terrain_type == TERRAIN_MUD) && (d->posCurr.y < 1)))
		{
			if ((quad->quadFlags & 0x1000) != 0)
			{
				d->normalVecUP.x = sps->Set2.normalVec[0];
				d->normalVecUP.y = sps->Set2.normalVec[1];
				d->normalVecUP.z = sps->Set2.normalVec[2];
				d->unkAA |= 8;
			}

			if (d->currBlockTouching == NULL)
			{
				d->currBlockTouching = quad;
				d->AxisAngle1_normalVec.x = sps->Set2.normalVec[0];
				d->AxisAngle1_normalVec.y = sps->Set2.normalVec[1];
				d->AxisAngle1_normalVec.z = sps->Set2.normalVec[2];
			}

			COLL_FIXED_PlayerSearch_UpdateLighting(sps, d, inst);
		}
	}

	if (d->quadBlockHeight + 0x8000 < d->posCurr.y)
	{
		d->terrainMeta2 = VehAfterColl_GetTerrain(TERRAIN_NONE);
	}

	if (d->posCurr.y < ((s32)level->ptr_mesh_info->bspRoot->box.min[1] - 0x40) * 0x100)
	{
		d->unkAA |= 1;
	}

	landingDelta = d->velocity.y - d->ySpeed;

	if ((d->currBlockTouching != NULL) && ((d->unkAA & 9) == 0) && (d->kartState != KS_MASK_GRABBED))
	{
		d->velocity.x += d->AxisAngle1_normalVec.x >> 1;
		d->velocity.y += d->AxisAngle1_normalVec.y >> 1;
		d->velocity.z += d->AxisAngle1_normalVec.z >> 1;
	}

	quad = d->currBlockTouching;

	d->xSpeed = d->velocity.x;
	d->ySpeed = d->velocity.y;
	d->zSpeed = d->velocity.z;

	if (quad == NULL)
	{
		goto DriverAirborne;
	}

	if (COLL_FIXED_PlayerSearch_CheckMaskGrabProgress(d, quad) != 0)
	{
		d->unkAA |= 1;
	}

	d->jump_LandingBoost = 0;
	d->actionsFlagSet &= 0xfff7ffbf;

	if ((d->unkAA & 8) == 0)
	{
		goto DriverAirborne;
	}

	if ((d->driverRankItemValue == 2) || ((gGT->gameMode2 & 0x80000) != 0))
	{
		d->currentTerrain = TERRAIN_ICE;
	}
	else
	{
		u8 terrainType = d->currBlockTouching->terrain_type;

		if ((terrainType != TERRAIN_ICE) && (d->currentTerrain == TERRAIN_ICE))
		{
			d->filler_short = 0xfec0;
		}

		d->currentTerrain = terrainType;
	}

	d->terrainMeta1 = VehAfterColl_GetTerrain(d->currentTerrain);
	d->terrainMeta2 = d->terrainMeta1;
	d->jump_CoyoteTimerMS = 0xa0;

	{
		u32 actions = d->actionsFlagSet;

		d->actionsFlagSet = actions | 1;

		if ((d->actionsFlagSetPrevFrame & 1) == 0)
		{
			s32 absLanding = COLL_FIXED_PlayerSearch_Abs(landingDelta);
			u32 volume;

			d->actionsFlagSet = actions | 0x83;
			d->filler_short = 0x140;

			volume = VehCalc_MapToRange(absLanding, 0x100, 0x3c00, 0x78, 0xfa);

			if (d->kartState != KS_MASK_GRABBED)
			{
				volume = (volume & 0xff) << 16;

				if ((d->actionsFlagSet & 0x10000) == 0)
				{
					volume |= 0x8080;
				}
				else
				{
					volume |= 0x1008080;
				}

				DECOMP_OtherFX_Play_LowLevel(7, 1, volume);
			}
		}
	}

	lerpFrames = 6;
	goto BlendNormal;

DriverAirborne:
	if (d->jump_CooldownMS != 0)
	{
		d->actionsFlagSet |= 0x80000;
	}

	if (d->jump_unknown != 0)
	{
		d->actionsFlagSet |= 0x40;
	}

	d->terrainMeta1 = VehAfterColl_GetTerrain(TERRAIN_NONE);
	d->currentTerrain = TERRAIN_NONE;
	d->actionsFlagSet &= ~1u;

	d->jump_LandingBoost += gGT->elapsedTimeMS;
	d->jump_CoyoteTimerMS -= gGT->elapsedTimeMS;
	if (d->jump_CoyoteTimerMS < 0)
	{
		d->jump_CoyoteTimerMS = 0;
	}

	lerpFrames = 7;
	if (d->jump_CoyoteTimerMS == 0)
	{
		d->jump_CooldownMS = 0;
		d->jump_unknown = 0;
	}

BlendNormal:
{
	s32 invFrames = 8 - lerpFrames;
	s32 normalX = (CollFixed_MulLo(lerpFrames, d->AxisAngle2_normalVec[0]) + CollFixed_MulLo(invFrames, d->normalVecUP.x)) >> 3;
	s32 normalY = (CollFixed_MulLo(lerpFrames, d->AxisAngle2_normalVec[1]) + CollFixed_MulLo(invFrames, d->normalVecUP.y)) >> 3;
	s32 normalZ = (CollFixed_MulLo(lerpFrames, d->AxisAngle2_normalVec[2]) + CollFixed_MulLo(invFrames, d->normalVecUP.z)) >> 3;

	if (d->hazardTimer > 0)
	{
		struct CollFixedPlayerTrig trig = COLL_FIXED_PlayerSearch_Trig(CollFixed_MulLo(d->hazardTimer, 0xc));
		s16 input[4] = {CollFixed_MulLo(trig.x, 0x19) >> 10, 0, CollFixed_MulLo(trig.z, 0x19) >> 10, 0};
		int output[3];

		gte_ldv0(input);
		gte_rtv0();
		gte_stlvnl(output);

		normalX += output[0];
		normalY += output[1];
		normalZ += output[2];
	}

	COLL_FIXED_PlayerSearch_NormalizeAxis2(d, normalX, normalY, normalZ);
}

	{
		struct CollFixedPlayerTrig trig = COLL_FIXED_PlayerSearch_Trig(d->angle);

		d->rotCurr.z = ratan2((CollFixed_MulLo(-d->AxisAngle2_normalVec[0], trig.z) + CollFixed_MulLo(d->AxisAngle2_normalVec[2], trig.x)) >> 12,
		                      d->AxisAngle2_normalVec[1]);
	}

	if (d->hazardTimer < 1)
	{
		if ((d->actionsFlagSet & 1) != 0)
		{
			s32 speed = COLL_FIXED_PlayerSearch_Abs(d->speed);

			if (speed > 0x1000)
			{
				s32 screenOffset = COLL_FIXED_PlayerSearch_Abs((s8)d->Screen_OffsetY);

				if ((screenOffset < 4) && ((d->terrainMeta1->flags & 1) != 0))
				{
					d->distanceFromGround = 4;
					goto UpdateGroundOffset;
				}
			}
		}

		d->distanceFromGround = 0;
	}
	else
	{
		s32 screenOffset = COLL_FIXED_PlayerSearch_Abs((s8)d->Screen_OffsetY);

		if (screenOffset < 4)
		{
			d->distanceFromGround = 4;

			if ((d->kartState != 3) && ((s8)d->Screen_OffsetY > 0))
			{
				DECOMP_OtherFX_Play(0x10, 1);
			}
		}
	}

UpdateGroundOffset:
	if (COLL_FIXED_PlayerSearch_Abs((s8)d->Screen_OffsetY) > 9)
	{
		d->distanceFromGround = 0;
	}

	if (d->distanceFromGround == 0)
	{
		s8 nextOffset = (s8)d->Screen_OffsetY - 4;

		if ((s8)d->Screen_OffsetY > 0)
		{
			d->Screen_OffsetY = nextOffset;

			if (nextOffset < 1)
			{
				d->Screen_OffsetY = 0;

				if ((d->terrainMeta1->flags & 0x20) != 0)
				{
					u32 soundFlags = ((d->actionsFlagSet & 0x10000) != 0) ? 0x1808080 : 0x808080;

					DECOMP_OtherFX_Play_LowLevel(d->terrainMeta1->sound, 0, soundFlags);
				}
			}
		}

		nextOffset = (s8)d->Screen_OffsetY - 4;
		d->Screen_OffsetY = nextOffset;

		if (nextOffset < 0)
		{
			d->Screen_OffsetY = 0;
		}
	}
	else
	{
		d->distanceFromGround--;
		d->Screen_OffsetY += 3;
	}

	if ((d->posCurr.y < -0x8000) && ((level->configFlags & 2) != 0))
	{
		d->unkAA |= 1;
	}

	if ((d->kartState != KS_MASK_GRABBED) && ((d->unkAA & 1) != 0) && (d->lastValid != NULL) && ((sdata->HudAndDebugFlags & 0x1000) == 0) &&
	    ((d->stepFlagSet & 8) == 0))
	{
		VehStuckProc_MaskGrab_Init(t, d);
	}
}
