struct UiElement2D
{
	s16 x;
	s16 y;
	s16 z;
	s16 scale;
};

struct UiElement3D
{
	// 0x0
	s16 rot[4];

	// 0x8
	MATRIX m;

	// 0x28
	SVec3 lightDir;
	s16 _pad_lightDir;

	// 0x30
	s16 vel[4];

	// 0x38 bytes
};

struct QuipStr
{
	s16 lngIndex;
	s16 flags;
	int priority;
};

struct QuipMeta
{
	struct QuipStr *ptrQuipStrCurr;
	struct QuipStr *ptrQuipStrNext;
	s16 conditionType;
	s16 flags;
	int threshold;
	int driverOffset;
	int dataSize;
};

_Static_assert(sizeof(struct UiElement2D) == 0x8);
_Static_assert(sizeof(struct UiElement3D) == 0x38);
_Static_assert(sizeof(struct QuipStr) == 0x8);
_Static_assert(sizeof(struct QuipMeta) == 0x18);
