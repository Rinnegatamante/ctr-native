#ifndef PLATFORM_H
#define PLATFORM_H

void Platform_Init(const char *title, int width, int height);
void Platform_Shutdown(void);
void Platform_InitScratchpad(void);
void Platform_BeginFrame(void);
void Platform_EndFrame(void);
int Platform_PollInput(void);
void Platform_InitFilesystem(const char *disc_image);

#endif
