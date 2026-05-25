#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002d67c-0x8002dc4c
void Audio_Update1(void)
{
	char i;
	int raceOrderIndex;
	s16 uVar1;
	u32 uVar2;
	struct Driver *d = 0;
	u32 maskTempo;
	int iVar7;
	struct GameTracker *gGT = sdata->gGT;

	switch (sdata->unkAudioState - 1)
	{
	case 5:
		if ((sdata->XA_State == 0) || (sdata->XA_Playing_Category != 0))
		{
			Audio_SetState_Safe(5);
		}
		break;
	case 7:
		Garage_LerpFX();
		break;
	case 8:
		if (sdata->XA_State == 0)
		{
			// 9 means intro cutscene
			// 10 means traffic lights
			// 11 means racing

			// Change state to traffic lights
			Audio_SetState_Safe(10);
		}
		break;
	case 9:
		// If traffic lights finish counting down
		// from 0x3840 to zero
		if (gGT->trafficLightsTimer < 1)
		{
			// 9 means intro cutscene
			// 10 means traffic lights
			// 11 means racing

			// Change State to 11, which means racing
			Audio_SetState_Safe(11);
		}
		break;
	case 10:
		Audio_SetMaskSong(0);

		// human driver in the lead
		for (i = 0; i < 8; i++)
		{
			d = gGT->driversInRaceOrder[i];

			if ((d != NULL) && ((d->actionsFlagSet & 0x100000) == 0))
			{
				break;
			}
			d = 0;
		}

		Voiceline_Update();

		Level_AmbientSound();

		// if race has more than 2 laps
		if ((2 < sdata->gGT->numLaps) &&
		    // if you are on 2nd to last lap
		    ((d->lapIndex == sdata->gGT->numLaps - 2U)) &&
		    // distToFinish is small
		    (d->distanceToFinish_curr < 9000))
		{
			Audio_SetState_Safe(12);
		}
		break;
	case 11:
		Audio_SetMaskSong(0);

		// human driver in the lead
		for (i = 0; i < 8; i++)
		{
			d = gGT->driversInRaceOrder[i];

			if ((d != NULL) && ((d->actionsFlagSet & 0x100000) == 0))
			{
				break;
			}
			d = 0;
		}

		// if need to XASeek
		if (((sdata->boolNeedXASeek != 0) && (sdata->XA_State == 0)) && (9 < gGT->frameTimer_MainFrame_ResetDB - sdata->XA_PauseFrame))
		{
			// far from finish line
			if (2000 < d->distanceToFinish_curr)
			{
				// (FINAL LAP! music)
				CDSYS_XASeek(1, 0, 6);
			}

			// dont need to XASeek
			sdata->boolNeedXASeek = 0;
		}

		Level_AmbientSound();

		// if driver is on final lap
		if (d->lapIndex == gGT->numLaps - 1U)
		{
			// Play final lap sound
			Audio_SetState_Safe(13);
		}
		break;
	case 12:
		maskTempo = 0;

		// if XA has been playing more than a second
		if (0xe1 < sdata->XA_CurrOffset)
		{
			Music_RaiseVolume();

			maskTempo = 20;
		}

		Audio_SetMaskSong(maskTempo);

		Level_AmbientSound();

		if (sdata->XA_State == 0)
		{
			Audio_SetState_Safe(14);
		}
		break;
	case 13:
		Audio_SetMaskSong(20);

		// human driver in the lead
		for (i = 0; i < 8; i++)
		{
			d = gGT->driversInRaceOrder[i];

			if ((d != NULL) && ((d->actionsFlagSet & 0x100000) == 0))
			{
				break;
			}
			d = 0;
		}

		Voiceline_Update();

		Level_AmbientSound();

		if (
		    // if driver's lap is the last lap
		    (d->lapIndex == gGT->numLaps - 1U) &&

		    // if finish line is close
		    (d->distanceToFinish_curr < 9000))
		{
			Audio_SetState_Safe(15);
		}

		break;
	case 14:
		raceOrderIndex = -1;

		// human driver in the lead
		for (i = 0; i < 8; i++)
		{
			d = gGT->driversInRaceOrder[i];

			if ((d != NULL) && (d->instSelf->thread->modelIndex == DYNAMIC_PLAYER))
			{
				raceOrderIndex = i;
				break;
			}
			d = 0;
		}

		// if need to XASeek
		if (((sdata->boolNeedXASeek != 0) && (sdata->XA_State == 0)) && (9 < gGT->frameTimer_MainFrame_ResetDB - sdata->XA_PauseFrame))
		{
			// far from finish line
			if (2000 < d->distanceToFinish_curr)
			{
				CDSYS_XASeek(1, 0, 4);
			}

			// dont need to XASeek
			sdata->boolNeedXASeek = 0;
		}

		Level_AmbientSound();

		// if the race is over for this racer
		if ((d->actionsFlagSet & 0x2000000) != 0)
		{
			// if did not just open N Tropy
			if ((gGT->gameModeEnd & NTROPY_JUST_OPENED) == 0)
			{
				// If you did not just beat N Tropy in Time Trial
				if ((gGT->gameModeEnd & NTROPY_JUST_BEAT) == 0)
				{
					// defeat music
					uVar1 = 5;

					if ((raceOrderIndex == 0) || ((gGT->gameMode1 & ADVENTURE_CUP) != 0) || ((gGT->gameMode2 & CUP_ANY_KIND) != 0))
					{
						DECOMP_OtherFX_Play(0x5f, 0);

						// victory (fanfare) music
						uVar1 = 4;
					}
				}

				// If you just beat N Tropy in Time Trial
				else
				{
					// Check if N Tropy has been beaten on all tracks
					uVar2 = GAMEPROG_CheckGhostsBeaten(1);

					// If there is a track where N Tropy has not been beaten
					if ((uVar2 & 0xffff) == 0)
					{
						// OtherFX_Play
						DECOMP_OtherFX_Play(0x5f, 0);

						// count how many times N Tropy has been beaten
						// during this playthrough
						iVar7 = sdata->nTropyVoiceCount;
						sdata->nTropyVoiceCount = iVar7 + 1;

						// pick an N Tropy XA voiceline
						uVar1 = data.nTropyXA[iVar7];

						// if beaten more than 5 times
						if (5 < sdata->nTropyVoiceCount)
						{
							// go back to 0
							sdata->nTropyVoiceCount = 0;
						}
					}

					// If N Tropy has been beaten on all tracks
					else
					{
						DECOMP_OtherFX_Play(0x5f, 0);

						// N Tropy Unlocked XA
						// "you've earned the right to choose me in character selection"
						uVar1 = 0x43;
					}
				}
			}

			// if n tropy just opened
			else
			{
				// OtherFX_Play
				DECOMP_OtherFX_Play(0x5f, 0);

				// N Tropy opened
				// "Think your fast eh, well, lets see if you can beat my fastest time"
				uVar1 = 0x46;
			}

			// desired XA
			sdata->desiredXA_RaceEndIndex = uVar1;

			Audio_SetState_Safe(0x10);
		}
		break;
	case 15:

		Level_AmbientSound();

		if (sdata->XA_State == 0)
		{
			Audio_SetMaskSong(0);
		}
	}
}
