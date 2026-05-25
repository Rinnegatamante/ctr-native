#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800abbb4-0x800abefc.
void RB_Player_KillPlayer(struct Driver *attacker, struct Driver *victim)
{
	struct GameTracker *gGT = sdata->gGT;
	u32 gameMode = gGT->gameMode1;
	u8 numPlyr = gGT->numPlyrCurrGame;

	if ((gameMode & BATTLE_MODE) == 0)
		return;

	if (attacker == NULL)
		return;

	if (victim == NULL)
		return;

	if ((gameMode & POINT_LIMIT) != 0)
	{
		int attackerTeam = attacker->BattleHUD.teamID;
		int victimTeam = victim->BattleHUD.teamID;

		if (victimTeam == attackerTeam)
		{
			int score = gGT->battleSetup.pointsPerTeam[victimTeam] - 1;

			if (score < -9)
				return;

			gGT->battleSetup.pointsPerTeam[victimTeam] = score;
			return;
		}

		int score = gGT->battleSetup.pointsPerTeam[attackerTeam] + 1;

		if (score < 100)
		{
			gGT->battleSetup.pointsPerTeam[attackerTeam] = score;
		}

		if (gGT->battleSetup.pointsPerTeam[attackerTeam] != gGT->battleSetup.killLimit)
			return;

		if ((gameMode & TIME_LIMIT) != 0)
			return;

		for (int i = 0; i < numPlyr; i++)
		{
			gGT->drivers[i]->actionsFlagSet |= ACTION_RACE_FINISHED;
		}
	}
	else
	{
		if ((gameMode & LIFE_LIMIT) == 0)
			return;

		int lives = victim->BattleHUD.numLives - 1;

		if (lives > 0)
		{
			victim->BattleHUD.numLives = lives;
			return;
		}

		s16 isTeamAlive[4];
		memset(isTeamAlive, 0, sizeof(isTeamAlive));

		int deadPlayers = 0;
		s16 teamsAlive = 0;

		victim->funcPtrs[0] = VehStuckProc_RIP_Init;
		victim->BattleHUD.numLives = 0;
		victim->actionsFlagSet |= ACTION_RACE_FINISHED;

		for (int i = 0; i < numPlyr; i++)
		{
			struct Driver *driver = gGT->drivers[i];

			if ((driver->actionsFlagSet & ACTION_RACE_FINISHED) == 0)
			{
				isTeamAlive[driver->BattleHUD.teamID] = 1;
			}
			else
			{
				deadPlayers++;
			}
		}

		int victimTeam = victim->BattleHUD.teamID;

		if (((gGT->battleSetup.teamFlags & (1 << victimTeam)) != 0) && (isTeamAlive[victimTeam] == 0))
		{
			int remainingPlayers = numPlyr - deadPlayers;

			if (remainingPlayers < 3)
			{
				gGT->standingsPoints[victimTeam * 3 + remainingPlayers]++;
			}

			gGT->battleSetup.finishedRankOfEachTeam[victimTeam] = remainingPlayers;
		}

		for (int team = 0; team < 4; team++)
		{
			if (((gGT->battleSetup.teamFlags & (1 << team)) != 0) && (isTeamAlive[team] != 0))
			{
				teamsAlive++;
			}
		}

		if (teamsAlive != 1)
			return;

		for (int i = 0; i < numPlyr; i++)
		{
			gGT->drivers[i]->actionsFlagSet |= ACTION_RACE_FINISHED;
		}
	}

	MainGameEnd_Initialize();
}
