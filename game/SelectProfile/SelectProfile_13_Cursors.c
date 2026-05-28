#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80048da0-0x80048de4.
void SelectProfile_MuteCursors(void)
{
	data.menuFourAdvProfiles.state |= MUTE_SOUND_OF_MOVING_CURSOR;
	data.menuGhostSelection.state |= MUTE_SOUND_OF_MOVING_CURSOR;
	data.menuWarning2.state |= MUTE_SOUND_OF_MOVING_CURSOR;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80048de4-0x80048e2c.
void SelectProfile_UnMuteCursors(void)
{
	data.menuFourAdvProfiles.state &= ~MUTE_SOUND_OF_MOVING_CURSOR;
	data.menuGhostSelection.state &= ~MUTE_SOUND_OF_MOVING_CURSOR;
	data.menuWarning2.state &= ~MUTE_SOUND_OF_MOVING_CURSOR;
}
