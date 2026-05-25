#include <common.h>

static u32 Voiceline_RequestPlay_NextAudioRNG(void)
{
	sdata->audioRNG = ((sdata->audioRNG >> 3) + sdata->audioRNG * 0x20000000) * 5 + 1;
	return sdata->audioRNG;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002cbe8-0x8002cf28
void Voiceline_RequestPlay(u32 voiceID, u32 characterID, u32 characterID2)
{
	u8 voiceType;
	u32 elapsedFrames;
	u32 canImmediate;
	u32 canQueue;

	if (voiceID >= 0x18)
		return;

	if (characterID >= 0x10)
		return;

	if (characterID2 >= 0x11)
		return;

	if ((sdata->gGT->gameMode1 & 0x200000) != 0)
		return;

	voiceType = data.voiceID[voiceID];

	if ((s32)voiceID >= 8)
	{
		u32 alreadyPlayed = sdata->timeSet1[characterID] & (1 << (voiceID & 0x1f));
		u32 rng = Voiceline_RequestPlay_NextAudioRNG();

		if (alreadyPlayed != 0)
		{
			rng &= 7;
		}
		else
		{
			rng &= 3;
		}

		if (rng != 0)
			return;
	}

	elapsedFrames = (u32)(sdata->gGT->frameTimer_MainFrame_ResetDB - sdata->timeSet2[characterID]);
	canImmediate = 0;
	if (elapsedFrames >= 0x3d)
	{
		canImmediate = voiceType < 2;
	}

	canQueue = 1;
	if (sdata->boolCanPlayVoicelines == 0)
	{
		canQueue = 0;
	}
	else if ((sdata->voicelineCooldown != 0) && (((u8 *)sdata->backupParams_FUN_8002cf28)[0xa] == characterID))
	{
		canQueue = 0;
	}
	else if (elapsedFrames < 0x3c)
	{
		canQueue = 0;
	}

	if (canQueue != 0)
	{
		if (canImmediate != 0)
		{
			u32 rng = Voiceline_RequestPlay_NextAudioRNG();

			canImmediate = 0;
			if ((rng & 1) != 0)
			{
				canQueue = 0;
				goto playImmediate;
			}
		}
	}
	else
	{
		if (canImmediate == 0)
			return;
	}

	if (canImmediate == 0)
		goto queueVoiceline;

playImmediate:
	if (voiceType == 0)
	{
		DECOMP_OtherFX_Play((characterID + 0x1c) & 0xffff, 2);
	}
	else if (voiceType == 1)
	{
		DECOMP_OtherFX_Play((characterID + 0x2c) & 0xffff, 2);
	}

	sdata->timeSet2[characterID] = sdata->gGT->frameTimer_MainFrame_ResetDB;
	return;

queueVoiceline:
	if (canQueue == 0)
		return;

	sdata->timeSet1[characterID] |= 1 << (voiceID & 0x1f);

	for (struct Item *item = sdata->Voiceline2.first; item != NULL; item = item->next)
	{
		u8 *itemBytes = (u8 *)item;

		if ((voiceID == (u32) * (s16 *)(itemBytes + 8)) && (characterID == itemBytes[0xa]))
			return;
	}

	struct Item *item = sdata->Voiceline1.first;
	if (item != NULL)
	{
		LIST_RemoveMember(&sdata->Voiceline1, item);
	}
	else
	{
		item = sdata->Voiceline2.last;
		if (item != NULL)
		{
			LIST_RemoveMember(&sdata->Voiceline2, item);
		}
	}

	LIST_AddFront(&sdata->Voiceline2, item);

	{
		u8 *itemBytes = (u8 *)item;

		itemBytes[0xa] = characterID;
		itemBytes[0xb] = characterID2;
		*(s16 *)(itemBytes + 8) = voiceID;
		*(s32 *)(itemBytes + 0xc) = sdata->gGT->timer;
	}
}
