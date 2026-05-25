#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002d2f4-0x8002d4cc
void Audio_SetState(u32 state)
{
	u8 XA_type;
	u8 XA_index;

	int iVar1;

	struct GameTracker *gGT = sdata->gGT;
	u16 level;

	XA_type = CDSYS_XA_TYPE_MUSIC;

	switch (state)
	{
	// stop/pause cseq music (main: case 2)
	case 1:
		sdata->boolNeedXASeek = 0;

		CDSYS_XAPauseRequest();

		Music_Restart();

		// erase backup, keep music, stop all fx
		DECOMP_howl_StopAudio(1, 0, 1);
		break;
	case 2:
	case 7:

		CseqMusic_StopAll();

		Music_Adjust(0, 0, 0, 0);

		break;
	case 5:

		CseqMusic_StopAll();

		level = gGT->levelID;

		// Level ID on Adventure Arena
		if (level - 0x19U < 5)
		{
			// convert levelID to a bitshifted flag
			Music_Adjust(0, 0, &sdata->advHubSongSet, 1 << (level - 0x19U));
		}
		break;
	case 9:
		XA_index = sdata->desiredXA_RaceIntroIndex;
		sdata->desiredXA_RaceIntroIndex += 1;
		sdata->desiredXA_RaceIntroIndex &= 3;
		goto PLAY_XA;

	case 10:

		Music_Stop();

		CseqMusic_StopAll();
		break;
	case 11:

		sdata->WrongWayDirection_bool = false;

		sdata->framesDrivingSameDirection = 0;

		Voiceline_ToggleEnable(1);
		break;

	case 12:

	// last lap, distToFinish < 9000
	case 15:

		Voiceline_ToggleEnable(0);

		sdata->boolNeedXASeek = 1;
		break;

	// if you are on last lap
	case 13:
		sdata->boolNeedXASeek = 0;

		Music_LowerVolume();

		// MUSIC_LAST_LAP
		XA_index = 6;

		goto PLAY_XA;
	case 14:

		sdata->WrongWayDirection_bool = false;

		sdata->framesDrivingSameDirection = 0;

		Music_RaiseVolume();

		Voiceline_ToggleEnable(1);

		break;
	case 16:
		sdata->boolNeedXASeek = 0;

		Music_Restart();

		// set XA
		XA_index = sdata->desiredXA_RaceEndIndex;

		if (61 < XA_index)
		{
			XA_type = CDSYS_XA_TYPE_EXTRA;
		}

	PLAY_XA:

		CDSYS_XAPlay(XA_type, XA_index);
	}
}
