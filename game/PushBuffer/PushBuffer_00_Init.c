#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800426f8-0x80042910.
void PushBuffer_Init(struct PushBuffer *pb, int id, int total)
{
#if BUILD == EurRetail
#define SIZEY_TOP 0x74
#define SIZEY_1P  0xEC
#define STARTY_2P 0x78
#else
#define SIZEY_TOP 0x6a
#define SIZEY_1P  0xD8
#define STARTY_2P 0x6e
#endif

	pb->fade_step = 0x88;
	pb->matrix_Proj.m[0][0] = 0x1c71;
	*(u8 *)&pb->cameraID = (u8)id;

	pb->fadeFromBlack_currentValue = 0x1000;
	pb->fadeFromBlack_desiredResult = 0x1000;

	pb->matrix_Proj.m[0][1] = 0;
	pb->matrix_Proj.m[0][2] = 0;
	pb->matrix_Proj.m[1][0] = 0;
	pb->matrix_Proj.m[1][1] = 0x1000;
	pb->matrix_Proj.m[1][2] = 0;
	pb->matrix_Proj.m[2][0] = 0;
	pb->matrix_Proj.m[2][1] = 0;
	pb->matrix_Proj.m[2][2] = 0x1000;
	pb->matrix_Proj.t[0] = 0;
	pb->matrix_Proj.t[1] = 0;
	pb->matrix_Proj.t[2] = 0;

	if (total == 1)
	{
		pb->rect.w = 0x200;
		pb->rect.h = SIZEY_1P;

		pb->distanceToScreen_PREV = 0x100;
		pb->distanceToScreen_CURR = 0x100;

		pb->aspectX = 4;
		pb->rect.x = 0;
		pb->rect.y = 0;
		pb->aspectY = 3;
		return;
	}

	if (total == 2)
	{
		if (id == 0)
		{
			pb->rect.w = 0x200;
			pb->rect.h = SIZEY_TOP;

			pb->distanceToScreen_PREV = 0x100;
			pb->distanceToScreen_CURR = 0x100;

			pb->aspectX = 8;
			pb->rect.x = 0;
			pb->rect.y = 0;
			pb->aspectY = 3;
			return;
		}

		if (id == 1)
		{
			pb->rect.y = STARTY_2P;
			pb->rect.w = 0x200;
			pb->rect.h = SIZEY_TOP;

			pb->distanceToScreen_PREV = 0x100;
			pb->distanceToScreen_CURR = 0x100;

			pb->aspectX = 8;
			pb->rect.x = 0;
			pb->aspectY = 3;
		}

		return;
	}

	if ((total < 3) || (total > 4))
		return;

	if (id == 0)
	{
		pb->rect.w = 0xfd;
		pb->rect.h = SIZEY_TOP;

		pb->distanceToScreen_PREV = 0x80;
		pb->distanceToScreen_CURR = 0x80;

		pb->aspectX = 4;
		pb->rect.x = 0;
		pb->rect.y = 0;
		pb->aspectY = 3;
		return;
	}

	if (id == 1)
	{
		pb->rect.x = 0x103;
		pb->rect.w = 0xfd;
		pb->rect.h = SIZEY_TOP;

		pb->distanceToScreen_PREV = 0x80;
		pb->distanceToScreen_CURR = 0x80;

		pb->aspectX = 4;
		pb->rect.y = 0;
		pb->aspectY = 3;
		return;
	}

	if (id == 2)
	{
		pb->rect.y = STARTY_2P;
		pb->rect.w = 0xfd;
		pb->rect.h = SIZEY_TOP;

		pb->distanceToScreen_PREV = 0x80;
		pb->distanceToScreen_CURR = 0x80;

		pb->aspectX = 4;
		pb->rect.x = 0;
		pb->aspectY = 3;
		return;
	}

	if (id == 3)
	{
		pb->rect.x = 0x103;
		pb->rect.y = STARTY_2P;
		pb->rect.w = 0xfd;
		pb->rect.h = SIZEY_TOP;

		pb->distanceToScreen_PREV = 0x80;
		pb->distanceToScreen_CURR = 0x80;

		pb->aspectX = 4;
		pb->aspectY = 3;
	}

	return;
}
