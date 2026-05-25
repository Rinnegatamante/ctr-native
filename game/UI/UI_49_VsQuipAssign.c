#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80054a78-0x80054bfc
void UI_VsQuipAssign(struct Driver *driver, struct QuipMeta *meta, struct Driver *bestDriver, int characterID)
{
	if (driver == NULL)
		return;

	if (((meta->flags & 4) != 0) && (driver != bestDriver))
		return;

	struct QuipStr *selected = meta->ptrQuipStrCurr;

	for (struct QuipStr *curr = selected + 1; curr < meta->ptrQuipStrNext; curr++)
	{
		if (selected->priority < curr->priority)
		{
			selected = curr;
			continue;
		}

		if ((curr->priority == selected->priority) && (((MixRNG_Scramble() >> 3) & 0xff) < 0x40))
		{
			selected = curr;
		}
	}

	struct QuipStr *oldQuip = (struct QuipStr *)driver->EndOfRaceComment_ptrQuip;

	if (oldQuip != NULL)
	{
		int oldPriority = oldQuip->priority;

		if (selected->priority <= oldPriority)
		{
			if (selected->priority != oldPriority)
				return;

			if (((MixRNG_Scramble() >> 3) & 0xff) > 0x3f)
				return;
		}

		if (oldQuip->priority < 0)
		{
			oldQuip->priority = -oldQuip->priority;
		}
	}

	if (selected->priority > 0)
	{
		selected->priority = -selected->priority;
	}

	driver->EndOfRaceComment_ptrQuip = (s16 *)selected;
	driver->EndOfRaceComment_characterID = characterID;

	sdata->gGT->timerEndOfRaceVS = ((sdata->gGT->gameMode1 & BATTLE_MODE) != 0) ? 150 : 300;
}

void DECOMP_UI_VsQuipAssign(struct Driver *driver, struct QuipMeta *meta, struct Driver *bestDriver, int characterID)
{
	UI_VsQuipAssign(driver, meta, bestDriver, characterID);
}
