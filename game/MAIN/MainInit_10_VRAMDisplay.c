#include <common.h>

#ifdef REBUILD_PC
#include <PsyX/PsyX_render.h>
#endif

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003c310-0x8003c41c; native wraps
// the VRAM page moves in a platform frame for presentation.
void MainInit_VRAMDisplay()
{
	RECT r;
	DR_MOVE move;

	s16 x[2];
	s16 y[2];

	x[0] = 0;
	x[1] = 0x100;

	y[0] = 0;
	y[1] = 0x128;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			r.x = x[i] + 0x200;
			r.y = 0x10c;
			r.w = 0x100;
			r.h = 0xd8;

			SetDrawMove(&move, &r, x[i], y[j]);

			move.tag |= 0xffffff;

			DrawOTag((u32 *)&move);
			DrawSync(0);
		}
	}

#ifdef REBUILD_PC
	Platform_BeginFrame();
	GR_PresentVRAMDisplay();
	Platform_EndFrame();
#endif
}
