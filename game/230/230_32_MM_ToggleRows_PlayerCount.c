#include <common.h>

// NOTE(aalhendi): ASM-verified against NTSC-U 926 overlay 230 0x800ad448-0x800ad560.
void MM_ToggleRows_PlayerCount()
{
	int i;
	struct MenuRow *row;

	for (i = 0; i < 2; i++)
	{
		row = &D230.rowsPlayers1P2P[i];

		// unlock row
		row->stringIndex &= 0x7fff;

		if ((MainFrame_HaveAllPads(i + 1) & 0xffff) == 0)
		{
			// lock row
			row->stringIndex |= 0x8000;
		}
	}

	for (i = 0; i < 3; i++)
	{
		row = &D230.rowsPlayers2P3P4P[i];

		// unlock row
		row->stringIndex &= 0x7fff;

		if ((MainFrame_HaveAllPads(i + 2) & 0xffff) == 0)
		{
			// lock row
			row->stringIndex |= 0x8000;
		}
	}
}
