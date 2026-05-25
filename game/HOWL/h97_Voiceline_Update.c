#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002d0f8-0x8002d2a8
void Voiceline_Update(void)
{
	struct GameTracker *gGT = sdata->gGT;

	if (sdata->boolCanPlayVoicelines == 0)
		return;

	if (sdata->voicelineCooldown != 0)
	{
		sdata->voicelineCooldown--;
		if (sdata->voicelineCooldown != 0)
			return;
	}

	if (sdata->XA_State != 0)
		return;

	if (sdata->boolCanPlayWrongWaySFX != 0)
	{
		if ((sdata->WrongWayDirection_bool != 0) && (sdata->framesDrivingSameDirection > 0x1e))
		{
			u32 voiceID;

			sdata->boolCanPlayWrongWaySFX = false;

			if (gGT->numPlyrCurrGame == 1)
			{
				if ((VehPickupItem_MaskBoolGoodGuy(gGT->drivers[0]) & 0xffff) == 0)
				{
					voiceID = 0x3d;
				}
				else
				{
					voiceID = 0x1e;
				}

				if (CDSYS_XAPlay(CDSYS_XA_TYPE_EXTRA, voiceID) == 0)
				{
					sdata->voicelineCooldown = 0x1e;
					return;
				}

				sdata->voicelineCooldown = (s16)(CDSYS_XAGetTrackLength(CDSYS_XA_TYPE_EXTRA, voiceID) / 5) + 0x1e;
				return;
			}
		}

		if (sdata->boolCanPlayWrongWaySFX != 0)
			goto playQueuedVoice;
	}

	if ((sdata->WrongWayDirection_bool == 0) && (sdata->framesDrivingSameDirection > 0x1e))
	{
		sdata->boolCanPlayWrongWaySFX = true;
	}

playQueuedVoice:
	if (sdata->Voiceline2.first != NULL)
	{
		struct Item *first = sdata->Voiceline2.first;

		LIST_RemoveMember(&sdata->Voiceline2, first);
		LIST_AddBack(&sdata->Voiceline1, first);
		Voiceline_StartPlay(first);
	}
}
