#include <common.h>

#ifdef REBUILD_PC
// 32mb RAM, so that PrimMem can fill the entirety
// of 24-bit address space (16mb max) and have enough
// RAM left over for the rest of the game (16mb more)
char memory[32 * 1024 * 1024];
#endif

void RB_EndOfFile();
void CS_EndOfFile();
extern char RB_NewEndFile[4];
void OVR_Region3();

void MEMPACK_Init(int ramSize)
{
	u32 startPtr;
	int packSize;

#ifdef REBUILD_PC

	// must be a 24-bit address
	// Visual Studio -> Properties -> Linker -> Advanced ->
	// Base Address, Randomized Base Address, Fixed Base Address
	startPtr = &memory[0];

	int boolValid = startPtr < 0x01000000;

	printf("[CTR] Where does memory starts? (%s) %08x\n", boolValid ? "GOOD" : "BAD", startPtr);

	packSize = 32 * 1024 * 1024;
	memset(startPtr, 0, packSize);

#else

	// Defragmentation requires a lower overlay size in bigfile,
	// which we dont have for 231, so we cheat by setting size
	// manually in MainMain after call to DECOMP_LOAD_ReadDirectory

	// RB_EndOfFile		800ba548	30 sectors (original game, deprecated by decomp)
	// Defragged 231	800b8c78	28 sectors
	// CS_EndOfFile		800b97fc	28 sectors (original game, current largest size)

	u32 Aligned231 = 28 * 0x800;
	// u32 Size231 = (u32)RB_NewEndFile - (u32)OVR_Region3;
	// u32 Aligned231 = ((Size231 + 0x7ff) & ~(0x7ff));
	// printf("\nMEMPACK_Init: %d sectors in 231\n\n", Aligned231/0x800);

	// Original game allocated 0x800 to stack,
	// but now Stack is relocated to kernel memory
	startPtr = ((u32)OVR_Region3 + Aligned231);
	packSize = ramSize - (int)(startPtr & 0xffffff);
#endif

	MEMPACK_NewPack(startPtr, packSize);
}
