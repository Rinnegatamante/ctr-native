#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800b0300-0x800b06ac
void CS_Podium_FullScene_Init(void)
{
	struct Instance *driverInstSelf;
	struct Thread *victoryCamThread;
	u32 podiumMusic;
	struct CsThreadInitData InitData;

	struct PosRot
	{
		SVec3 pos;
		SVec3 rot;
	};

	struct PosRot *posRot;

	struct GameTracker *gGT = sdata->gGT;

	// assume cutscene did not manipulate audio
	D233.CutsceneManipulatesAudio = 0;

	// Make a backup of FX volume, clamp to 0x100
	D233.FXVolumeBackup = howl_VolumeGet(0);
	D233.FXVolumeBackup &= 0xff;

	// Make a backup of Music volume, clamp to 0x100
	D233.MusicVolumeBackup = howl_VolumeGet(1);
	D233.MusicVolumeBackup &= 0xff;

	// Make a backup of Voice volume, clamp to 0x100
	D233.VoiceVolumeBackup = howl_VolumeGet(2);
	D233.VoiceVolumeBackup &= 0xff;

	// Cutscene is now starting
	D233.isCutsceneOver = 0;
	D233.cutsceneState = CS_CAMERA_PAN;

	D233.PodiumInitUnk3 = 0;

	driverInstSelf = gGT->drivers[0]->instSelf;

	D233.PodiumInitUnk2 = 0;

	driverInstSelf->flags |= HIDE_MODEL;

	VehPhysProc_FreezeEndEvent_Init(driverInstSelf->thread, gGT->drivers[0]);

	// Number of Winners = 1
	// this means Draw Confetti on one window
	gGT->numWinners = 1;

	// Set winnerIndex[0] to 0, to draw
	// confetti on the first pushBuffer
	gGT->winnerIndex[0] = 0;

	gGT->confetti.numParticles_max = 200;
	gGT->confetti.unk2 = 200;
	gGT->hudFlags &= 0xfe;

	// Draw Confetti
	gGT->renderFlags |= 4;

	gGT->gameMode2 |= VEH_FREEZE_PODIUM;

	// position and rotation of podium scene
	// Y coordinate (podiumPos.y) has added height
	posRot = (struct PosRot *)gGT->level1->ptrSpawnType2_PosRot[1].posCoords;
	InitData.podiumPos.x = posRot->pos.x;
	InitData.podiumPos.y = posRot->pos.y + 0x80;
	InitData.podiumPos.z = posRot->pos.z;
	InitData.rot.x = posRot->rot.x;
	InitData.rot.y = posRot->rot.y;
	InitData.rot.z = posRot->rot.z;

	// convert 3 rotation shorts into rotation matrix
	ConvertRotToMatrix((MATRIX *)&InitData.local_30, &InitData.rot);
	// Move position of trophy girl
	gte_SetLightMatrix(&InitData.local_30);

	// CameraDC, this makes the camera stop following you as it does while racing, it must be zero to follow you
	gGT->cameraDC[0].cameraMode = 3;

	// if someone placed third
	if (gGT->podium_modelIndex_Third != '\0')
	{
		InitData.characterPos.x = 299;
		InitData.characterPos.y = 0xffab;
		InitData.characterPos.z = 0;

		// create thread for "third"
		CS_Thread_Init(gGT->podium_modelIndex_Third, &R233.s_third[0], (void *)&InitData, 0x600, 0);
	}

	// if someone placed second
	if (gGT->podium_modelIndex_Second != '\0')
	{
		InitData.characterPos.x = 0xfed5;
		InitData.characterPos.y = 0xffd6;
		InitData.characterPos.z = 0;

		// create thread for "second"
		CS_Thread_Init(gGT->podium_modelIndex_Second, &R233.s_second[0], (void *)&InitData, 0x200, 0);
	}

	InitData.characterPos.x = 0;
	InitData.characterPos.y = 0;
	InitData.characterPos.z = 0;

	// create thread for "first"
	CS_Thread_Init(gGT->podium_modelIndex_First, &R233.s_first[0], (void *)&InitData, 0, 0);

	InitData.characterPos.x = 0x1a8;
	InitData.characterPos.y = 0xff80;
	InitData.characterPos.z = 0x140;

	// create thread for trophy girl (internally called "tawna")
	CS_Thread_Init(gGT->podium_modelIndex_tawna, &R233.s_tawna[0], (void *)&InitData, -0x2aa, 0);

	CS_Podium_Prize_Init(gGT->podiumRewardID, &R233.s_prize[0], (void *)&InitData);

	CS_Podium_Stand_Init((void *)&InitData);

	// PROC_BirthWithObject
	// 0x4 = size
	// 0 = no relation to param4
	// 0x300 flag = SmallStackPool
	// 0xf = camera thread bucket
	victoryCamThread = (struct Thread *)PROC_BirthWithObject(0x4030f, (void *)CS_Camera_ThTick_Podium, R233.s_victorycam, NULL);

	// if it allocated correctly
	if (victoryCamThread != 0)
	{
		// initialize first "s16" of the object to zero
		*(s16 *)victoryCamThread->object = 0;
	}

	// change victory music based on who is first in the podium
	switch (gGT->podium_modelIndex_First - 0x7e)
	{
	// Crash, Coco, Fake Crash
	case 0:
	case 3:
	case 0xE:
		// Crash's music
		podiumMusic = 10;
		break;

	// Cortex, NGin, NTrophy
	case 1:
	case 4:
	case 0xC:
		// Cortex's music
		podiumMusic = 8;
		break;

	// Polar Pura
	case 6:
	case 7:
		// Polar and Pura's music
		podiumMusic = 7;
		break;

	// pinstripe kjoe
	case 8:
	case 0xb:
		// Pinstripe's music
		podiumMusic = 0xb;
		break;

	// papu, roo, penta
	case 9:
	case 10:
	case 0xD:
		// Ripper Roo's music
		podiumMusic = 9;
		break;

	// Tiny, Dingo, Oxide
	default:
		// Default music is Tiny Tiger's
		podiumMusic = 0xc;
		break;
	}

	CDSYS_XAPlay(0, podiumMusic);

	return;
}
