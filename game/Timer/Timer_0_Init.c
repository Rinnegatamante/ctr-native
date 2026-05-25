#include <common.h>

void Timer_Init()
{
	EnterCriticalSection();
	StopRCnt(DescRC + 1);
	SetRCnt(DescRC + 1, 0xffff, 0x2000);
	StartRCnt(DescRC + 1);
	ExitCriticalSection();
}
