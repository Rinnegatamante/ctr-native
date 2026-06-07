#include <common.h>

struct VehPhysForceTrigPair
{
	s32 sin;
	s32 cos;
};

static u32 VehPhysForce_ReadWord(const void *base, int offset)
{
	return *(const u32 *)(const void *)((const char *)base + offset);
}

static int VehPhysForce_CountLeadingSignBits(s32 value)
{
	u32 bits = (u32)value;
	u32 sign = bits >> 31;
	int count = 0;

	while (count < 32 && (((bits >> (31 - count)) & 1) == sign))
		count++;

	return count;
}

static struct VehPhysForceTrigPair VehPhysForce_TrigAngleSinCos(int angle)
{
	u32 packed = VehPhysForce_ReadWord(&data.trigApprox[angle & 0x3ff], 0);
	struct VehPhysForceTrigPair pair;

	if ((angle & 0x400) == 0)
	{
		pair.sin = (s16)packed;
		pair.cos = (s16)(packed >> 16);

		if ((angle & 0x800) != 0)
		{
			pair.sin = -pair.sin;
			pair.cos = -pair.cos;
		}
	}
	else
	{
		pair.sin = (s16)(packed >> 16);
		pair.cos = (s16)packed;

		if ((angle & 0x800) != 0)
			pair.sin = -pair.sin;
		else
			pair.cos = -pair.cos;
	}

	return pair;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8005f89c-0x8005fb4c.
void VehPhysForce_RotAxisAngle(MATRIX *m, s16 *normVec, s16 angle)
{
	s32 normalX = normVec[0];
	s32 normalY = normVec[1];
	s32 normalZ = normVec[2];
	struct VehPhysForceTrigPair trig = VehPhysForce_TrigAngleSinCos(angle);
	s32 normalXSq = normalX * normalX;
	s32 normalZSq = normalZ * normalZ;
	s32 crossXZ = normalX * -normalZ;
	s32 denom = normalXSq + normalZSq;
	s32 scaledSinY = (trig.sin * normalY) >> 12;
	s32 scaledCosY = (trig.cos * normalY) >> 12;
	s32 outX = (s16)scaledSinY;
	s32 outY;
	s32 outZ = (s16)scaledCosY;

	m->m[0][1] = (s16)normalX;
	m->m[1][1] = (s16)normalY;
	m->m[2][1] = (s16)normalZ;

	if (denom == 0)
	{
		s32 dot = trig.sin * normalX + trig.cos * normalZ;

		if (normalY < 0)
			outX = -outX;

		outY = -dot >> 12;
	}
	else
	{
		int shift = 0x14 - VehPhysForce_CountLeadingSignBits(denom);
		s32 sinRemainder;
		s32 cosRemainder;
		s32 divX;
		s32 divZ;
		s32 dot;

		if (shift > 0)
		{
			normalXSq >>= shift;
			normalZSq >>= shift;
			crossXZ >>= shift;
			denom >>= shift;
		}

		sinRemainder = trig.sin - scaledSinY;
		cosRemainder = trig.cos - scaledCosY;
		divX = (sinRemainder * normalZSq + cosRemainder * crossXZ) / denom;
		divZ = (sinRemainder * crossXZ + cosRemainder * normalXSq) / denom;
		dot = trig.sin * normalX + trig.cos * normalZ;

		outX = (s16)(outX + (s16)divX);
		outY = -dot >> 12;
		outZ = (s16)(outZ + (s16)divZ);
	}

	m->m[0][2] = (s16)outX;
	m->m[1][2] = (s16)outY;
	m->m[2][2] = (s16)outZ;

	// NOTE(aalhendi): Retail uses GTE `OP(sf=12)` with the normal vector and the generated
	// third column to derive the first matrix column. The partial register
	// writes are intentional; callers observe this exact GTE state.
	CTC2((u32)normalX, 0);
	CTC2((u32)normalY, 2);
	CTC2((u32)normalZ, 4);
	MTC2((u32)(s32)m->m[0][2], 9);
	MTC2((u32)(s32)m->m[1][2], 10);
	MTC2((u32)(s32)m->m[2][2], 11);
	gte_op12();
	m->m[0][0] = (s16)MFC2_S(25);
	m->m[1][0] = (s16)MFC2_S(26);
	m->m[2][0] = (s16)MFC2_S(27);
}
