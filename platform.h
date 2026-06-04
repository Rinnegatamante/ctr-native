#ifndef PLATFORM_H
#define PLATFORM_H

struct PlatformMempackArena
{
	void *base;
	void *start;
	void *endOfMemory;
	int size;
	int backingSize;
	int lowAddressValid;
};

void Platform_Init(const char *title, int width, int height);
void Platform_Shutdown(void);
void Platform_InitScratchpad(void);
const struct PlatformMempackArena *Platform_InitMempackArena(void);
void Platform_BeginFrame(void);
int Platform_BeginScene(void);
void Platform_EndScene(void);
void Platform_EndFrame(void);
void Platform_PresentVRAMDisplay(void);
void Platform_PollHostEvents(void);
int Platform_PollInput(void);

#endif
