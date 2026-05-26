#include <common.h>

#if defined(CTR_NATIVE)
// NOTE(aalhendi): Native does not expose EXE rdata; this mirrors 0x80011620.
static const char s_rectMenuTimeFormat[] = "%ld:%ld%ld:%ld%ld";
#define RECTMENU_TIME_FORMAT s_rectMenuTimeFormat
#else
#define RECTMENU_TIME_FORMAT (&rdata.s_timeString[0])
#endif

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80044ff8-0x80045134.
u8 *RECTMENU_DrawTime(int milliseconds)
{
	// 32 is added to milliseconds every frame,
	// 960 per second, the rest is basic math

	char *str = &sdata->ghostStrTrackTime[0];

	// build a string
	sprintf(

	    str,

	    // Format
	    // Minute:Seconds:Milliseconds
	    RECTMENU_TIME_FORMAT,

	    milliseconds / 0xe100,              // minutes
	    (milliseconds / 0x2580) % 6,        // seconds / 10
	    (milliseconds / 0x3c0) % 10,        // seconds
	    ((milliseconds * 10) / 0x3c0) % 10, // milliseconds / 10
	    ((milliseconds * 100) / 0x3c0) % 10 // milliseconds
	);

	return (u8 *)str;
}

#undef RECTMENU_TIME_FORMAT
