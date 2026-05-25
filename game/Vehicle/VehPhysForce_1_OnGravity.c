#include <common.h>

enum
{
	VEH_PHYS_FORCE_ACTION_PREVENT_ACCEL = 0x8,
	VEH_PHYS_FORCE_ACTION_AIRBORNE = 0x80000,
	VEH_PHYS_FORCE_ACTION_800000 = 0x800000,
	VEH_PHYS_FORCE_ACTION_BACK_SKID = 0x800,
	VEH_PHYS_FORCE_ACTION_FRONT_SKID = 0x1000,
	VEH_PHYS_FORCE_QUAD_LOW_GRAVITY = 0x2,
};

static int VehPhysForce_OnGravity_Abs(int value)
{
	return value < 0 ? -value : value;
}

static Vec3 VehPhysForce_OnGravity_RotateVector(const MATRIX *m, s16 vx, s16 vy, s16 vz)
{
	Vec3 out;

	out.x = ((int)m->m[0][0] * vx + (int)m->m[0][1] * vy + (int)m->m[0][2] * vz) >> 12;
	out.y = ((int)m->m[1][0] * vx + (int)m->m[1][1] * vy + (int)m->m[1][2] * vz) >> 12;
	out.z = ((int)m->m[2][0] * vx + (int)m->m[2][1] * vy + (int)m->m[2][2] * vz) >> 12;

	return out;
}

static Vec3 VehPhysForce_OnGravity_RotateVectorTranspose(const MATRIX *m, s16 vx, s16 vy, s16 vz)
{
	Vec3 out;

	out.x = ((int)m->m[0][0] * vx + (int)m->m[1][0] * vy + (int)m->m[2][0] * vz) >> 12;
	out.y = ((int)m->m[0][1] * vx + (int)m->m[1][1] * vy + (int)m->m[2][1] * vz) >> 12;
	out.z = ((int)m->m[0][2] * vx + (int)m->m[1][2] * vy + (int)m->m[2][2] * vz) >> 12;

	return out;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005e214-0x8005ea60
void VehPhysForce_OnGravity(struct Driver *driver, Vec3 *velocity)
{
	int elapsedTimeMS = sdata->gGT->elapsedTimeMS;
	Vec3 localVelocity = VehPhysForce_OnGravity_RotateVectorTranspose(&driver->matrixMovingDir, (s16)velocity->x, (s16)velocity->y, (s16)velocity->z);
	int originalLocalZ = localVelocity.z;
	int gravityY = -driver->const_Gravity;
	struct QuadBlock *underDriver = driver->underDriver;

	// NOTE(aalhendi): Retail does not branch before reading this flag. Native
	// can reach this frame after TeleportSelf clears underDriver and before
	// fixed collision repopulates it; PS1 low-RAM behavior is no low-gravity
	// flag, while host C would crash on a null dereference.
	if ((underDriver != NULL) && ((underDriver->quadFlags & VEH_PHYS_FORCE_QUAD_LOW_GRAVITY) != 0))
	{
		gravityY = (-driver->const_Gravity * 41) / 100;
	}

	gravityY = (gravityY * elapsedTimeMS) >> 5;
	Vec3 localGravity = VehPhysForce_OnGravity_RotateVectorTranspose(&driver->matrixMovingDir, 0, (s16)gravityY, 0);

	if (((localGravity.z < 0) && (driver->unk_offset3B2 > 0)) || ((localGravity.z > 0) && (driver->unk_offset3B2 < 0)))
	{
		localGravity.z = 0;
	}

	u32 actionsFlagSet = driver->actionsFlagSet;
	int speedApprox = driver->speedApprox;
	int baseSpeed = driver->baseSpeed;
	if ((actionsFlagSet & VEH_PHYS_FORCE_ACTION_PREVENT_ACCEL) || ((baseSpeed > 0) && (speedApprox < 0)) ||
	    ((driver->baseSpeed < 0) && (driver->speedApprox > 0)))
	{
		localGravity.x = 0;
		localGravity.z = 0;
	}

	int originalLocalX = localVelocity.x;
	int originalLocalY = localVelocity.y;
	int localX = originalLocalX + localGravity.x;
	int localY = originalLocalY + localGravity.y;
	int localZ = originalLocalZ + localGravity.z;

	int maxForwardSpeed = driver->fireSpeed + driver->unk47E;
	if ((localZ > maxForwardSpeed) && (originalLocalZ < maxForwardSpeed))
	{
		localZ = maxForwardSpeed;
	}

	int minForwardSpeed = driver->fireSpeed - (driver->unk47E >> 1);
	if ((localZ < minForwardSpeed) && (originalLocalZ > minForwardSpeed))
	{
		localZ = minForwardSpeed;
	}

	int maxPerpendicularSpeed = driver->unk480;
	if ((localX > maxPerpendicularSpeed) && (originalLocalX < maxPerpendicularSpeed))
	{
		localX = maxPerpendicularSpeed;
	}

	int minPerpendicularSpeed = -maxPerpendicularSpeed;
	if ((localX < minPerpendicularSpeed) && (originalLocalX > minPerpendicularSpeed))
	{
		localX = minPerpendicularSpeed;
	}

	u32 terrainFlags = driver->terrainMeta1->flags;
	int terminalVelocity = driver->const_TerminalVelocity;
	if ((localY < 0) && ((terrainFlags & 0x80) != 0) && (originalLocalY < -0x100))
	{
		terminalVelocity = 0x100;
		originalLocalY = -0x100;
	}

	int clampedLocalY = localY;
	if (localY > terminalVelocity)
	{
		clampedLocalY = originalLocalY;
		if (originalLocalY < terminalVelocity)
		{
			clampedLocalY = terminalVelocity;
		}
	}

	int minTerminalVelocity = -terminalVelocity;
	if (clampedLocalY < minTerminalVelocity)
	{
		clampedLocalY = originalLocalY;
		if (originalLocalY > minTerminalVelocity)
		{
			clampedLocalY = minTerminalVelocity;
		}
	}

	localY = clampedLocalY;

	if (driver->kartState == KS_MASK_GRABBED)
	{
		localX = 0;
		localZ = 0;
	}
	else if (((driver->actionsFlagSetPrevFrame & ACTION_TOUCH_GROUND) != 0) || (driver->kartState == KS_BLASTED) ||
	         ((driver->unknowndriverBaseSpeed < driver->speedApprox) && (driver->terrainMeta2->unk_0x8 < 0x100)))
	{
		int perpendicularFriction;
		int forwardFriction;

		if ((actionsFlagSet & VEH_PHYS_FORCE_ACTION_PREVENT_ACCEL) == 0)
		{
			if (baseSpeed == 0)
			{
				perpendicularFriction = driver->const_NoPedalFriction_Perpendicular;
				forwardFriction = driver->const_NoPedalFriction_Forward;

				if (driver->driverRankItemValue == 5)
				{
					perpendicularFriction = driver->const_BrakeFriction << 4;
					forwardFriction = perpendicularFriction;
				}
			}
			else
			{
				int absSpeedApprox = VehPhysForce_OnGravity_Abs(speedApprox);
				if ((absSpeedApprox < 0x301) || (((baseSpeed < 1) || (speedApprox >= 0)) && ((baseSpeed >= 0) || (speedApprox < 1))))
				{
					if (driver->kartState == KS_DRIFTING)
					{
						perpendicularFriction = driver->const_DriftCurve;
						forwardFriction = driver->const_DriftFriction;
					}
					else
					{
						perpendicularFriction = driver->const_PedalFriction_Perpendicular;
						forwardFriction = driver->const_PedalFriction_Forward;

						if (absSpeedApprox > 0x300)
						{
							int absBaseSpeed = VehPhysForce_OnGravity_Abs(baseSpeed);
							if (absSpeedApprox < (absBaseSpeed >> 1))
							{
								actionsFlagSet |= VEH_PHYS_FORCE_ACTION_BACK_SKID;
							}
						}
					}
				}
				else
				{
					perpendicularFriction = driver->const_PedalFriction_Perpendicular;
					forwardFriction = driver->const_BrakeFriction;

					if (absSpeedApprox > 0x300)
					{
						actionsFlagSet |= VEH_PHYS_FORCE_ACTION_BACK_SKID;
					}
				}
			}
		}
		else
		{
			int absSpeedApprox = VehPhysForce_OnGravity_Abs(speedApprox);
			if (absSpeedApprox > 0x300)
			{
				actionsFlagSet |= VEH_PHYS_FORCE_ACTION_BACK_SKID;
			}

			perpendicularFriction = driver->const_BrakeFriction;
			if (driver->driverRankItemValue == 5)
			{
				perpendicularFriction <<= 4;
				forwardFriction = perpendicularFriction;
			}
			else if (driver->kartState == KS_BLASTED)
			{
				perpendicularFriction = (perpendicularFriction * 3) >> 2;
				forwardFriction = perpendicularFriction;
			}
			else
			{
				forwardFriction = perpendicularFriction;
				if (driver->kartState == KS_SPINNING)
				{
					perpendicularFriction >>= 1;
					forwardFriction = perpendicularFriction;
				}
			}
		}

		perpendicularFriction = (perpendicularFriction * elapsedTimeMS) >> 5;
		forwardFriction = (forwardFriction * elapsedTimeMS) >> 5;

		int terrainFrictionScale = driver->terrainMeta1->unk_0x20[0];
		if (terrainFrictionScale != 0x100)
		{
			perpendicularFriction = (terrainFrictionScale * perpendicularFriction) >> 8;
			forwardFriction = (terrainFrictionScale * forwardFriction) >> 8;
		}

		int terrainTimer = driver->filler_short;
		if (terrainTimer < 0)
		{
			int absLocalX = localX;
			if (terrainTimer == -0x140)
			{
				absLocalX = VehPhysForce_OnGravity_Abs(localX);
				perpendicularFriction = absLocalX >> 1;
			}
			else
			{
				perpendicularFriction += (perpendicularFriction * driver->const_unk444) >> 8;
				if (perpendicularFriction < 0)
				{
					perpendicularFriction = 0;
				}

				absLocalX = VehPhysForce_OnGravity_Abs(localX);
			}

			if (absLocalX > 0)
			{
				actionsFlagSet |= VEH_PHYS_FORCE_ACTION_BACK_SKID | VEH_PHYS_FORCE_ACTION_FRONT_SKID;
				GAMEPAD_ShockForce1(driver, 4, 0x7f);
				GAMEPAD_ShockFreq(driver, 4, 0);
			}

			terrainTimer += elapsedTimeMS;
			if (terrainTimer > 0)
			{
				terrainTimer = 0;
			}
			driver->filler_short = (s16)terrainTimer;
		}
		else if (terrainTimer > 0)
		{
			terrainTimer -= elapsedTimeMS;
			if (terrainTimer < 0)
			{
				terrainTimer = 0;
			}

			perpendicularFriction += (perpendicularFriction * driver->const_unk444) >> 8;
			driver->filler_short = (s16)terrainTimer;
			if (perpendicularFriction < 0)
			{
				perpendicularFriction = 0;
			}
		}

		if (((actionsFlagSet & VEH_PHYS_FORCE_ACTION_800000) == 0) && ((terrainFlags & 0x80) != 0))
		{
			int absSideSpeed = localX >> 3;
			if (absSideSpeed < 0)
			{
				absSideSpeed = -absSideSpeed;
			}
			if (perpendicularFriction < absSideSpeed)
			{
				perpendicularFriction = absSideSpeed;
			}

			int minForwardFriction = 0;
			if ((localZ == 0) || (baseSpeed == 0) || ((localZ ^ baseSpeed) >= 0))
			{
				if (((baseSpeed <= localZ) || (localZ > 0)) && ((localZ <= baseSpeed) || (localZ < 0)))
				{
					goto APPLY_TERRAIN_FRICTION;
				}

				minForwardFriction = VehPhysForce_OnGravity_Abs(localZ - baseSpeed) >> 1;
			}
			else
			{
				minForwardFriction = VehPhysForce_OnGravity_Abs(localZ >> 1);
			}

			if (forwardFriction < minForwardFriction)
			{
				forwardFriction = minForwardFriction;
			}
		}

	APPLY_TERRAIN_FRICTION:
	{
		int xFriction = perpendicularFriction;
		if ((terrainFlags & 0x100) != 0)
		{
			xFriction = (perpendicularFriction * 3) >> 2;
			if (xFriction < forwardFriction)
			{
				xFriction = forwardFriction;
			}
		}

		localX = VehCalc_InterpBySpeed(localX, xFriction, 0);
		localZ = VehCalc_InterpBySpeed(localZ, forwardFriction, 0);
	}
	}

	*velocity = VehPhysForce_OnGravity_RotateVector(&driver->matrixMovingDir, (s16)localX, (s16)localY, (s16)localZ);
	driver->actionsFlagSet = actionsFlagSet;

	if ((actionsFlagSet & VEH_PHYS_FORCE_ACTION_AIRBORNE) == 0)
	{
		if ((driver->boolFirstFrameSinceRevEngine != 0) && (localZ != 0))
		{
			driver->forwardDir = (localZ < 0) ? -1 : 1;
			driver->boolFirstFrameSinceRevEngine = 0;
			goto CHECK_ROLLBACK_FROM_PREVIOUS_DIRECTION;
		}

		if (originalLocalZ < 0)
		{
			goto SET_FORWARD_DIRECTION_IF_NONNEGATIVE;
		}

		if (localZ < 0)
		{
			driver->forwardDir = -1;
		}

		if (originalLocalZ > 0)
		{
			goto CHECK_ROLLBACK_FROM_FORWARD;
		}

	SET_FORWARD_DIRECTION_IF_NONNEGATIVE:
		if (localZ >= 0)
		{
			driver->forwardDir = 1;
		}
	}

CHECK_ROLLBACK_FROM_PREVIOUS_DIRECTION:
	if (originalLocalZ >= 0)
	{
		goto CHECK_ROLLBACK_FROM_FORWARD;
	}

	if (localZ <= 0)
	{
		return;
	}
	goto START_ROLLBACK;

CHECK_ROLLBACK_FROM_FORWARD:
	if (localZ < 0)
	{
		goto START_ROLLBACK;
	}
	if (originalLocalZ > 0)
	{
		return;
	}
	if (localZ <= 0)
	{
		return;
	}

START_ROLLBACK:
	if (driver->StartRollback_0x280 != 0)
	{
		driver->unknownTraction++;
	}
	driver->StartRollback_0x280 = 0x280;
}
