#include <common.h>

void Timer_Destroy()
{
	EnterCriticalSection();
	StopRCnt(DescRC + 1);
	ExitCriticalSection();
}
