#include <common.h>

void MainInit_JitPoolsNew(struct GameTracker *gGT)
{
	char numPlyr;
	u32 gameMode;
	struct JitPool *pool;
	u32 uVar5;
	u32 uVar7;
	u32 uVar9;
	int *pointer;

	// game mode
	gameMode = gGT->gameMode1;
	numPlyr = gGT->numPlyrCurrGame;

	// Adventure Arena, 1P No Weapons
	uVar7 = 0x800;

	if (
	    // If you're not in Adventure Arena
	    ((gameMode & ADVENTURE_ARENA) == 0) && (
	                                               // All racing gameplay
	                                               uVar7 = 0x1000,

	                                               // If you're in main menu
	                                               (gameMode & MAIN_MENU) != 0))
	{
		// Main Menu, bare minimum
		uVar7 = 0x400;
	}

	// Adventure Arena, 1P No Weapons
	uVar9 = 0x800;

	// If you're not in Adventure Arena
	if ((gameMode & ADVENTURE_ARENA) == 0)
	{
		// All racing gameplay
		if ((gameMode & MAIN_MENU) == 0)
		{
			uVar9 = 0x1000;
		}

		else
		{
			// Main Menu, bare minimum
			uVar9 = 0x400;

			if (gGT->levelID == ADVENTURE_GARAGE)
			{
				uVar9 = 0x800;
			}
		}
	}


	// add a bookmark
	MEMPACK_PushState();

#ifdef CTR_INTERNAL
	fprintf(stderr, "=== JitPoolsNew: gameMode=0x%08X levelID=%d numPlyr=%d uVar7=0x%X uVar9=0x%X ===\n", gameMode, gGT->levelID, numPlyr, uVar7, uVar9);
#endif


	// normally maxed at 96
	int numThread = uVar9 * 3 >> 7;

	// ThreadPool
	JitPool_Init(&gGT->JitPools.thread, numThread, sizeof(struct Thread), /*"ThreadPool"*/ 0);

#ifdef CTR_INTERNAL
	fprintf(stderr, "  thread:       %3d items x 0x%X bytes\n", numThread, (u32)sizeof(struct Thread));
#endif


	// FIX(aalhendi): removed numMedium > 20 cap, not in SCUS ASM
	int numMedium = uVar7 >> 7;

	// MediumStackPool
	// ASM: li a2, 136 (0x88) at 0x8003b544
	// FIX: was sizeof(Item)+sizeof(WarpPad)=104, ASM uses 136 (+32/item)
	JitPool_Init(&gGT->JitPools.mediumStack, numMedium, 136, 0);

#ifdef CTR_INTERNAL
	fprintf(stderr, "  mediumStack:  %3d items x 0x%X bytes\n", numMedium, 136);
#endif


	// normally maxed at 100
	int numSmall = uVar7 * 0x19 >> 10;

	// SmallStackPool
	// ASM: li a2, 72 (0x48) at 0x8003b52c
	// FIX: was sizeof(Item)+sizeof(UiElement3D)=64, ASM uses 72 (+8/item)
	JitPool_Init(&gGT->JitPools.smallStack, numSmall, 72, 0);

#ifdef CTR_INTERNAL
	fprintf(stderr, "  smallStack:   %3d items x 0x%X bytes\n", numSmall, 72);
#endif


	// normally maxed at 128
	int numInstance = uVar9 >> 5;

	// Optimization tech
	if (numInstance == 128)
	{
		if (gGT->numPlyrCurrGame == 1)
		{
			// Do NOT touch Crystal Challenge, AdvHub, Cutscene,
			// only apply to race tracks: TimeTrial/Relic/Arcade
			if (gGT->levelID <= TURBO_TRACK)
			{
				// Except Dingo Canyon,
				// This track is why we need 128 in the first place,
				// Rest of tracks are MORE than 16 behind Dingo Canyon
				if (gGT->levelID != DINGO_CANYON)
				{
					numInstance -= 16;
				}
			}
		}

		else
		{
			// Apply to VS and Battle
			if (gGT->levelID <= LAB_BASEMENT)
			{
				numInstance -= 16;
			}
		}
	}

	// InstancePool
	JitPool_Init(&gGT->JitPools.instance, numInstance, sizeof(struct Instance) + (sizeof(struct InstDrawPerPlayer) * numPlyr),
	             /*"InstancePool"*/ 0);

#ifdef CTR_INTERNAL
	fprintf(stderr, "  instance:     %3d items x 0x%X bytes\n", numInstance, (u32)(sizeof(struct Instance) + (sizeof(struct InstDrawPerPlayer) * numPlyr)));
#endif


	int numDriver = uVar7 >> 9;
	if (gGT->numPlyrCurrGame == 2)
		numDriver = 6;
	if (gGT->numPlyrCurrGame > 2)
		numDriver = 4;
	if ((gameMode & TIME_TRIAL) != 0)
		numDriver = 3;
	if ((gameMode & MAIN_MENU) != 0)
		numDriver = 4;

	// ASM: li a2, 1648 (0x670) at 0x8003b574
	// FIX: was sizeof(Item)+sizeof(Driver)=1600, ASM uses 1648 (+48/item)
	JitPool_Init(&gGT->JitPools.largeStack, numDriver, 1648, 0);
	JitPool_Init(&gGT->JitPools.rain, numDriver, sizeof(struct RainLocal), /*"RainPool"*/ 0);


	// FIX(aalhendi): removed numParticle > 120 cap, not in SCUS ASM
	int numParticle = uVar7 >> 5;

	JitPool_Init(&gGT->JitPools.particle, numParticle, sizeof(struct Particle), /*"ParticlePool"*/ 0);
	JitPool_Init(&gGT->JitPools.oscillator, numParticle, 0x18, /*"OscillatorPool"*/ 0);


#ifdef REBUILD_PS1
	// original CTR code, still used for
	// REBUILD_PS1 and REBUILD_PC cause those
	// builds dont have OG game's bloatful RDATA
	gGT->ptrRenderBucketInstance = MEMPACK_AllocMem(uVar9 /*,"RENDER_BUCKET_INSTANCE"*/);
#else
	// save 0x400 - 0x1000 bytes
	// when compiling with OG game's RDATA
	// then expand PrimMem in 60fps,
	// add 148 bytes cause of MATH_Sin relocated
	gGT->ptrRenderBucketInstance = (void *)((int)148 + (int)&rdata.s_STATIC_GNORMALZ[0]);
#endif


	// ===========================================


	// FIX(aalhendi): load pool.free.first (the first ITEM), not the pool struct itself.
	// Old code started from the pool struct, which corrupted free.count on the first iter
	// by writing a ptr val into it.

	// small, medium, large
	for (int i = 0; i < 3; i++)
	{
		struct JitPool *pool = (struct JitPool *)((char *)&gGT->JitPools.smallStack + (sizeof(struct JitPool) * i));

		// start from first item in free list, NOT the pool struct
		pointer = (int *)pool->free.first;

		// loop through all items, write self-pointer at data offset
		while (pointer != (int *)0x0)
		{
			*(int **)(pointer + 2) = pointer + 2;
			pointer = (int *)*pointer;
		}
	}

	for (int i = 0; i < numPlyr; i++)
	{
		data.PtrClipBuffer[i] = MEMPACK_AllocMem(MainDB_GetClipSize(gGT->levelID, numPlyr) << 2
		                                         /*,"Clip Buffer"*/);
	}
}
