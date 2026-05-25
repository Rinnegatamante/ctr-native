#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002d554-0x8002d67c
void Audio_SetMaskSong(u32 tempo)
{
	char i;
	u8 isMaskUsed;
	u32 songID;
	struct GameTracker *gGT = sdata->gGT;
	u32 gameMode = gGT->gameMode1;

	// Assume no player is using a mask
	isMaskUsed = false;

	for (i = 0; i < gGT->numPlyrCurrGame; i++)
	{
		// if player is using mask weapon
		if ((gGT->drivers[i]->actionsFlagSet & 0x800000) != 0)
		{
			// There is at least one player using a mask
			isMaskUsed = true;
		}
	}

	// If any player is using a mask
	if (isMaskUsed)
	{
		// Uka song is playing
		songID = 2;

		if (
		    // If Uka song is playing
		    ((gameMode & UKA_SONG) != 0) ||

		    (
		        // Aku song is playing
		        songID = 1,

		        // If Aku song is playing
		        (gameMode & AKU_SONG) != 0))
		{
			Music_Adjust(songID, tempo, 0, 0);
		}
	}

	// If no players are using mask
	else
	{
		Music_Adjust(0, tempo, 0, 0);

		if ((gameMode & (UKA_SONG | AKU_SONG)) != 0)
		{
			gGT->gameMode1 &= ~(UKA_SONG | AKU_SONG);
		}
	}
}
