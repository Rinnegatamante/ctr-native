#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002a678-0x8002a6cc
u32 SongPool_CalculateTempo(s16 const60, s16 tpqn, s16 bpm)
{
	return (((tpqn * bpm) / 60) << 0x10) / const60;
}
