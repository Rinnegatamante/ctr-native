#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003eaac-0x8003eae0
u32 MixRNG_GetValue(int param_1)
{
	return (param_1 * 0x6255 + 0x3619U) & 0xffff;
}
