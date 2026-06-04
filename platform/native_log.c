#include "platform/native_log.h"

#include <stdarg.h>
#include <stdio.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

static FILE *s_logStream = NULL;

static void Platform_LogWrite(FILE *consoleStream, const char *text)
{
	FILE *stream = (consoleStream != NULL) ? consoleStream : stdout;

#ifdef _WIN32
	OutputDebugStringA(text);
#endif

	fputs(text, stream);

	if (s_logStream != NULL)
		fputs(text, s_logStream);
}

static void Platform_LogV(FILE *consoleStream, const char *fmt, va_list args)
{
	char text[4096];
	int written = vsnprintf(text, sizeof(text), fmt, args);

	if (written < 0)
		return;

	text[sizeof(text) - 1] = '\0';
	Platform_LogWrite(consoleStream, text);
}

void Platform_LogInit(const char *appName)
{
	char filename[128];
	int written = snprintf(filename, sizeof(filename), "%s.log", appName);

	if ((written < 0) || ((size_t)written >= sizeof(filename)))
	{
		fprintf(stderr, "[CTR Native] Error: log filename is too long\n");
		return;
	}

	s_logStream = fopen(filename, "wb");

	if (s_logStream == NULL)
		fprintf(stderr, "[CTR Native] Error: cannot create log file '%s'\n", filename);
}

void Platform_LogShutdown(void)
{
	Platform_LogWarn("---- LOG CLOSED ----\n");

	if (s_logStream != NULL)
		fclose(s_logStream);

	s_logStream = NULL;
}

void Platform_LogFlush(void)
{
	if (s_logStream != NULL)
		fflush(s_logStream);
}

void Platform_Log(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	Platform_LogV(stdout, fmt, args);
	va_end(args);
}

void Platform_LogWarn(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	Platform_LogV(stdout, fmt, args);
	va_end(args);
}

void Platform_LogError(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	Platform_LogV(stderr, fmt, args);
	va_end(args);
}
