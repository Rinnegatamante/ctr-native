#include <common.h>

#ifdef REBUILD_PC
struct
{
	int a;
	s16 b1, b2, c, d, e, f;
} commands;
#endif

void MainInit_VRAMClear()
{
	DRAWENV drawEnv;

#ifndef REBUILD_PC
	struct
	{
		int a;
		s16 b1, b2, c, d, e, f;
	} commands;
#endif

	SetDefDrawEnv(&drawEnv, 0, 0, 0x400, 0x200);
	drawEnv.dfe = '\x01';
	PutDrawEnv(&drawEnv);

	commands.a = 0x3ffffff;
	commands.b1 = 0;
	commands.b2 = 0x200;
	commands.c = 0;
	commands.d = 0;
	commands.e = 0x3ff;
	commands.f = 0x1ff;
	DrawOTag((u32 *)&commands);

	commands.d = 0x1ff;
	commands.f = 1;
	DrawOTag((u32 *)&commands);
}
