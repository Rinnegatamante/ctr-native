
struct CsThreadInitData
{
	// podium position
	// X Y Z and ?
	s16 podiumPos[4];

	// character position
	// X Y Z and ?
	s16 characterPos[4];

	// rotation (for both)
	// X Y Z and ?
	s16 rot[4];

	// matrix
	u32 local_30;
	u32 local_2c;
	u32 local_28;
	u32 local_24;
	u32 local_20;
};

struct Prize
{
	// 0x0
	s16 posStart[4];

	// 0x8
	s16 rot[4];

	// 0x10
	s16 posEnd[4];

	// 0x18
	s16 unk[4];

	// 0x20
	s16 vel[3];

	// 0x26
	s16 frameMax; // set in ThTick1

	// 0x28
	s16 frameMax2;

	// 0x2a
	s16 frameCurr;

	// 0x2c -- size of struct
};

struct unknown233
{
	void *unknown1;
	int unknown2;
	int unknown3;
};

union CsOpcodeArg
{
	int i;
	u32 u;
	char *ptr;
};

struct CsOpcodeMeta
{
	s16 opcode;
	s16 animIndex;
	s16 frameStart;
	s16 frameEnd;
	union CsOpcodeArg arg0; // shorts 4/5
	union CsOpcodeArg arg1; // shorts 6/7
	s16 rotStart;
	s16 rotEnd;
};

struct CutsceneObj
{
	// 0x0
	struct IconGroup *ptrIcons;
	// 0x4
	s16 unk4;
	// 0x6
	s16 unk6;

	/* 0x8
	jittery number? color?
	usually has value of 0x2e808080 */
	int unk8;

	// 0xC
	s16 unk_C;
	// 0xE
	s16 unk_E;

	// 0x10
	int *metadata;

	// 0x14
	s16 unk14;

	// 0x16
	// determines time or frame animation
	u16 flags;

	int unk18;

	s16 unk1c;
	s16 unk1e;
	s16 unk20;
	s16 unk22;
	s16 unk24;
	s16 unk26;
	u16 unk28;

	// 0x2a
	s16 scaleSpeed;
	// 0x2c
	s16 desiredScale;

	struct
	{
		// 0x2e
		s16 textPos[2];

		// 0x32
		// index to subtitle text in LNG
		// -1 to disable
		s16 lngIndex;

		// 0x34
		s16 font;

		// 0x36
		s16 colors;

	} Subtitles;


	// 0x38
	char *currOpcode[2];
	// 0x40
	char *prevOpcode;

	// 0x44
	char particleID;
	// 0x45
	char unk45;
	// 0x46
	char unk46;
	// 0x47
	u8 animIndex;

	// 0x48
	int *frameOverrideRoot;

	// 0x4c
	struct CsOpcodeMeta decodedOpcode;
};

#ifndef CTR_NATIVE
_Static_assert(sizeof(struct CsOpcodeMeta) == 0x14);
_Static_assert(OFFSETOF(struct CutsceneObj, frameOverrideRoot) == 0x48);
_Static_assert(OFFSETOF(struct CutsceneObj, decodedOpcode) == 0x4c);
_Static_assert(sizeof(struct CutsceneObj) == 0x60);
#endif

enum BOSS_CUTSCENE_ORDER
{
	// Gemstone
	OXIDE_TROPHIES,
	PINSTRIPE_BEAT,

	// Beach
	ROO_START,
	ROO_BEAT,

	// ruins
	PAPU_START,
	PAPU_BEAT,

	// glacier
	KJOE_START,
	KJOE_BEAT,

	// citadel
	PINSTRIPE_START,

	// 0x9
	OXIDE_RELICS_GEMSTONE,
	OXIDE_RELICS_BEACH,
	OXIDE_RELICS_RUINS,
	OXIDE_RELICS_GLACIER,
	OXIDE_RELICS_CITADEL,

	// 0xE
	BOSS_CUTSCENE_COUNT,
};

struct BossCutsceneData
{
	// 0x0
	int vrmFile_UNUSED;
	int headFile;
	int bodyFile;

	// Unused, cause it does model->id
	// to get the model index anyway
	int modelIndex_unused;

	// 0x10
	int opcode;

	// 0x14
	s16 camPos[4];
	s16 camRot[4];

	// 0x24
	s16 bossPos[4];
	s16 bossRot[4];

	// 0x34
};

extern struct
{
	char fill_beginning[4];

	char s_spawn[8];
	char s_g_dancer[16];

	char fill_strings[0x290];

	// CS_Podium_FullScene_Init
	char s_podium[8];
	// 0x800abca4
	char s_third[8];
	char s_second[8];
	char s_first[8];
	char s_tawna[8];
	char s_prize[8];
	char s_victorycam[16];

	char fill_strings2[0x3c];

	// CS_Thread_LInB
	char s_introguy[12];
	char s_introcam[12];

	// Naughty Dog crate intro thread strings
	char s_box1[8];
	char s_box2[8];
	char s_box2_bottom[16];
	char s_box2_front[16];
	char s_box2_A[8];
	char s_box3[8];
	char s_code[8];
	char s_glow[8];
	char s_lid[4];
	char s_lidb[8];
	char s_lidc[8];
	char s_lidd[8];
	char s_lid2[8];
	char s_kart0[8];
	char s_kart1[8];
	char s_kart2[8];
	char s_kart3[8];
	char s_kart6[8];
	char s_kart7[8];


	// TODO: Divide OVR_233 into 'real' sections
	char fill1[0x4DA8];


	// 800b0b7c
	int VertSplitLine;

	// 800b0b80
	int boolLoadNextSwap;

	// 800b0b84
	int boolStartToSkip;

	// 800b0b88
	int bossCutsceneIndex;

	// 800b0b8c
	int CutsceneManipulatesAudio;

	// 800b0b90
	// 120 bytes
	struct unknown233 whateverThisMeans[10];


	// TODO: Divide OVR_233 into 'real' sections
	char fill2[0x864];


	// 800b146c
	struct unknown233 pointerToWhateverThisMeans[10];


	char fill3_beforeTawnaScripts[0x2d0];

	// 800b17b4
	char script_tawnaNormal[0x28];

	// 800b17dc
	char script_tawnaCredits[0x164c];

	// 800b2e28
	char script_default[0x18];

	// 800b2e40
	char script_dingofire[0x38];

	// 800b2e78
	char *danceFirstScripts[0x10];

	// 800b2eb8
	char danceOtherOpcodeData[0x12d4];

	// 800b418c
	char *danceOtherScripts[0x10];

	// 800b41cc
	char introModelOpcodeData[0x3b0];

	// 800b457c
	char *introModelScripts[0x10];

	// 800b45bc
	char introCutsceneOpcodeData[0x11c];

	// 800b46d8
	// NOTE(aalhendi): Retail PTR_DAT_800b45bc_800b46d8.
	char *introCutsceneOpcodes[9];

	// 800b46fc
	char introEndingOpcodeData[0x50];

	// 800b474c
	char creditsCutsceneOpcodeData[0x1dc];

	// 800b4928
	// NOTE(aalhendi): Retail PTR_DAT_800b474c_800b4928.
	char *creditsCutsceneOpcodes[20];

	// 800b4978
	// ND crate intro script starts at offset 0x18.
	char creditsOpcodeData[0x40];

	// 800b49b8
	char boxAndAdvCharSelectOpcodeData[0x10c4];

	// 800b5a7c
	// NOTE(aalhendi): Retail PTR_DAT_800b49b8_800b5a7c.
	char *boxModelScripts[0x2b];

	// 800b5b28
	// NOTE(aalhendi): Retail PTR_DAT_800b4fe4_800b5b28.
	char *advCharSelectSelectOpcodes[8];

	// 800b5b48
	// NOTE(aalhendi): Retail PTR_DAT_800b5024_800b5b48.
	char *advCharSelectDeselectOpcodes[8];

	char fill3_afterAdvCharSelect[0x8];

	// 800b5b70
	char cs_initMatrixData[0x17c0];

	// 800b7330
	// NOTE(aalhendi): Retail PTR_DAT_800b5b70_800b7330.
	struct
	{
		void *data;
		int count;
	} cs_initMatrixTable[4];

	// 800b7350
	char cs_initMatrixBool;

	// 800b7351
	char fill3_afterInitMatrix_beforeClearBox[0x123];

	// 800b7474
	Color introClearBoxColor;

	// 800b7478
	RECT introClearBoxRect;

	// 800b7480
	s16 creditsDancerRotOffset[4];


	// 800B7488
	struct BossCutsceneData bossCS[0xE];

	// 800b7760
	int isCutsceneOver;
	// 800b7764
	int PodiumInitUnk2;
	// 800b7768
	s16 FXVolumeBackup;
	// 800b776a
	s16 MusicVolumeBackup;
	// 800b776c
	s16 VoiceVolumeBackup;

	s16 volumeunknown;

	// 800b7770
	int PodiumInitUnk3;
	// 800b7774
	int cutsceneState;

	// 800b7778
	struct Model *ptrModelBossHead;
	struct Model *ptrModelBossBody;

	// 800b7780

} OVR_233;

// NOTE(aalhendi): Layout-verified for live overlay-233 references used by the audited cutscene thread path.
// Retail base is NTSC-U 926 0x800ab9f0.
#define OVR233_LAYOUT_ASSERT(ELEMENT, OFFSET, SIZE)                 \
	_Static_assert(OFFSETOF(typeof(OVR_233), ELEMENT) == (OFFSET)); \
	_Static_assert(sizeof(((typeof(OVR_233) *)0)->ELEMENT) == (SIZE))

_Static_assert(sizeof(void *) == 4);
OVR233_LAYOUT_ASSERT(s_spawn, 0x4, 0x8);
OVR233_LAYOUT_ASSERT(s_g_dancer, 0xc, 0x10);
OVR233_LAYOUT_ASSERT(s_introguy, 0x328, 0xc);
OVR233_LAYOUT_ASSERT(s_introcam, 0x334, 0xc);
OVR233_LAYOUT_ASSERT(VertSplitLine, 0x518c, 0x4);
OVR233_LAYOUT_ASSERT(boolLoadNextSwap, 0x5190, 0x4);
OVR233_LAYOUT_ASSERT(boolStartToSkip, 0x5194, 0x4);
OVR233_LAYOUT_ASSERT(bossCutsceneIndex, 0x5198, 0x4);
OVR233_LAYOUT_ASSERT(CutsceneManipulatesAudio, 0x519c, 0x4);
OVR233_LAYOUT_ASSERT(whateverThisMeans, 0x51a0, 0x78);
OVR233_LAYOUT_ASSERT(pointerToWhateverThisMeans, 0x5a7c, 0x78);
OVR233_LAYOUT_ASSERT(script_tawnaNormal, 0x5dc4, 0x28);
OVR233_LAYOUT_ASSERT(script_tawnaCredits, 0x5dec, 0x164c);
OVR233_LAYOUT_ASSERT(script_default, 0x7438, 0x18);
OVR233_LAYOUT_ASSERT(script_dingofire, 0x7450, 0x38);
OVR233_LAYOUT_ASSERT(danceFirstScripts, 0x7488, 0x40);
OVR233_LAYOUT_ASSERT(danceOtherOpcodeData, 0x74c8, 0x12d4);
OVR233_LAYOUT_ASSERT(danceOtherScripts, 0x879c, 0x40);
OVR233_LAYOUT_ASSERT(introModelOpcodeData, 0x87dc, 0x3b0);
OVR233_LAYOUT_ASSERT(introModelScripts, 0x8b8c, 0x40);
OVR233_LAYOUT_ASSERT(introCutsceneOpcodeData, 0x8bcc, 0x11c);
OVR233_LAYOUT_ASSERT(introCutsceneOpcodes, 0x8ce8, 0x24);
OVR233_LAYOUT_ASSERT(introEndingOpcodeData, 0x8d0c, 0x50);
OVR233_LAYOUT_ASSERT(creditsCutsceneOpcodeData, 0x8d5c, 0x1dc);
OVR233_LAYOUT_ASSERT(creditsCutsceneOpcodes, 0x8f38, 0x50);
OVR233_LAYOUT_ASSERT(creditsOpcodeData, 0x8f88, 0x40);
OVR233_LAYOUT_ASSERT(boxAndAdvCharSelectOpcodeData, 0x8fc8, 0x10c4);
OVR233_LAYOUT_ASSERT(boxModelScripts, 0xa08c, 0xac);
OVR233_LAYOUT_ASSERT(advCharSelectSelectOpcodes, 0xa138, 0x20);
OVR233_LAYOUT_ASSERT(advCharSelectDeselectOpcodes, 0xa158, 0x20);
OVR233_LAYOUT_ASSERT(cs_initMatrixData, 0xa180, 0x17c0);
OVR233_LAYOUT_ASSERT(cs_initMatrixTable, 0xb940, 0x20);
OVR233_LAYOUT_ASSERT(cs_initMatrixBool, 0xb960, 0x1);
OVR233_LAYOUT_ASSERT(introClearBoxColor, 0xba84, 0x4);
OVR233_LAYOUT_ASSERT(introClearBoxRect, 0xba88, 0x8);
OVR233_LAYOUT_ASSERT(creditsDancerRotOffset, 0xba90, 0x8);
OVR233_LAYOUT_ASSERT(bossCS, 0xba98, 0x2d8);
OVR233_LAYOUT_ASSERT(isCutsceneOver, 0xbd70, 0x4);
OVR233_LAYOUT_ASSERT(PodiumInitUnk2, 0xbd74, 0x4);
OVR233_LAYOUT_ASSERT(FXVolumeBackup, 0xbd78, 0x2);
OVR233_LAYOUT_ASSERT(MusicVolumeBackup, 0xbd7a, 0x2);
OVR233_LAYOUT_ASSERT(VoiceVolumeBackup, 0xbd7c, 0x2);
OVR233_LAYOUT_ASSERT(PodiumInitUnk3, 0xbd80, 0x4);
OVR233_LAYOUT_ASSERT(cutsceneState, 0xbd84, 0x4);
OVR233_LAYOUT_ASSERT(ptrModelBossHead, 0xbd88, 0x4);
OVR233_LAYOUT_ASSERT(ptrModelBossBody, 0xbd8c, 0x4);
_Static_assert(sizeof(OVR_233) == 0xbd90);

#undef OVR233_LAYOUT_ASSERT

// starts at 800b8598,
// last byte of Garage Functions
struct OVR233_Garage
{
	// 800b8598
	struct RectMenu menuGarage;

	// 800b85c4
	int numFramesMax_GarageMove;
	int padding1;

	// 800b85cc
	int numFramesMax_Zoom;

	// 800b85d0
	int fovMin;

	// 800b85d4
	int fovMax;

	// 800b85d8
	s16 unusedArr_garageChars[8];

	// 800b85e8
	s16 barLen[3];

	// 800b85ee
	s16 unusedFrameCount;

	// 800b85f0
	s16 unusedArr_lngIndex[4];

	// 800b85f8
	s16 barStat[3 * 4];

	// 800b8610
	int unusedArr_Colors[3];

	// 800b861c
	int barColors[7];

	// 800b8638
	s16 numFramesCurr_GarageMove;

	// 800b863a
	s16 numFramesCurr_ZoomIn;

	// 800b863c
	s16 numFramesCurr_ZoomOut;

	// 800b863e
	s16 delayOneSecond;

	// 800b8640
	s16 boolSelected;
	s16 padding2;

	// === End of Garage Data ===
	// Credits RDATA starts next byte
};

// NOTE(aalhendi): Layout-verified pass 3 for garage data. Retail base is
// NTSC-U 926 0x800b8598.
#define OVR233_GARAGE_ASSERT(ELEMENT, OFFSET, SIZE)                      \
	_Static_assert(OFFSETOF(struct OVR233_Garage, ELEMENT) == (OFFSET)); \
	_Static_assert(sizeof(((struct OVR233_Garage *)0)->ELEMENT) == (SIZE))

OVR233_GARAGE_ASSERT(menuGarage, 0x0, 0x2c);
OVR233_GARAGE_ASSERT(numFramesMax_GarageMove, 0x2c, 0x4);
OVR233_GARAGE_ASSERT(padding1, 0x30, 0x4);
OVR233_GARAGE_ASSERT(numFramesMax_Zoom, 0x34, 0x4);
OVR233_GARAGE_ASSERT(fovMin, 0x38, 0x4);
OVR233_GARAGE_ASSERT(fovMax, 0x3c, 0x4);
OVR233_GARAGE_ASSERT(unusedArr_garageChars, 0x40, 0x10);
OVR233_GARAGE_ASSERT(barLen, 0x50, 0x6);
OVR233_GARAGE_ASSERT(unusedFrameCount, 0x56, 0x2);
OVR233_GARAGE_ASSERT(unusedArr_lngIndex, 0x58, 0x8);
OVR233_GARAGE_ASSERT(barStat, 0x60, 0x18);
OVR233_GARAGE_ASSERT(unusedArr_Colors, 0x78, 0xc);
OVR233_GARAGE_ASSERT(barColors, 0x84, 0x1c);
OVR233_GARAGE_ASSERT(numFramesCurr_GarageMove, 0xa0, 0x2);
OVR233_GARAGE_ASSERT(numFramesCurr_ZoomIn, 0xa2, 0x2);
OVR233_GARAGE_ASSERT(numFramesCurr_ZoomOut, 0xa4, 0x2);
OVR233_GARAGE_ASSERT(delayOneSecond, 0xa6, 0x2);
OVR233_GARAGE_ASSERT(boolSelected, 0xa8, 0x2);
_Static_assert(sizeof(struct OVR233_Garage) == 0xac);

#undef OVR233_GARAGE_ASSERT

extern struct OVR233_Garage gGarage;

#if 0
800b8644 - CREDITS RDATA
800b8668 - first byte of credits func (CREDITS TEXT)
#endif

// === Credits Functions Here ===

// 800b9488 - first byte after last func (CREDITS BSS)

struct CreditsLevHeader
{
	int size;
	s16 numStrings;
	s16 unused_06;

	// char* ptrStrings[0];
};

#define CREDITSHEADER_GETSTRINGS(x) ((u32)x + sizeof(struct CreditsLevHeader))

#ifndef CTR_NATIVE
_Static_assert(OFFSETOF(struct CreditsLevHeader, numStrings) == 0x4);
_Static_assert(sizeof(struct CreditsLevHeader) == 0x8);
#endif

struct CreditsObj
{
	// 800b94bc (000)
	struct Model *creditGhostModel[5]; // duplicates

	// 800b94d0 (014)
	struct Instance *creditGhostInst[5];

	// 800b94e4 (028)
	struct
	{
		char data[0x80];
	} data_0x80_0x5[5];

	// 800b9764 (2a8)
	struct
	{
		char data[0x18];
	} data_0x18_0x5[5];

	// 800b97dc (320)
	s16 countdown;

	// 800b97de (322)
	s16 unused_322;

	// 800b97e0 (324)
	int unk;

	// 800b97e4 (328)
	struct Instance *creditDanceInst; // base for copies

	// 800b97e8 (32c)
	s16 credits_posY;

	// 800b97ea (32e)
	s16 unused_32e;

	// 800b97ec (330)
	char *credits_topString;

	// 800b97f0 (334)
	char *epilogue_topString;

	// 800b97f4 (338)
	char *epilogue_nextString;

	// 800b97f8 (33c)
	s16 epilogueCount200;

	// 800b97fa (33e)
	s16 epiloguePosX; // unused
};

#ifndef CTR_NATIVE
_Static_assert(OFFSETOF(struct CreditsObj, countdown) == 0x320);
_Static_assert(OFFSETOF(struct CreditsObj, credits_posY) == 0x32c);
_Static_assert(OFFSETOF(struct CreditsObj, credits_topString) == 0x330);
_Static_assert(sizeof(struct CreditsObj) == 0x340);
#endif

struct Ovr233_Credits_BSS
{
	// 800b9488
	s16 creditGhost_Pos[4];

	// 800b9490
	s16 unused_Pos[4];

	// 800b9498
	int creditText_PosX;

	// 800b949c
	struct Thread *CreditThread;

	// 800b94a0
	struct Thread *DancerThread;

	// 800b94a4
	struct Instance *dancerInst_invisible;

	// 800b94a8
	s16 numStrings;

	// 800b94aa
	s16 unused_94aa;

	// 800b94ac
	char **ptrStrings;

	// 800b94b0
	s16 boolAllBlue;

	// 800b94b2
	s16 unused_94b2;

	// 800b94b4
	int unused[2];

	// 800b94bc - CreditsObj (0x340 large)
	struct CreditsObj creditsObj;
};

// NOTE(aalhendi): Layout-verified pass 3 for credits BSS. Retail base is
// NTSC-U 926 0x800b9488.
#define OVR233_CREDITS_OBJ_ASSERT(ELEMENT, OFFSET, SIZE)              \
	_Static_assert(OFFSETOF(struct CreditsObj, ELEMENT) == (OFFSET)); \
	_Static_assert(sizeof(((struct CreditsObj *)0)->ELEMENT) == (SIZE))

OVR233_CREDITS_OBJ_ASSERT(creditGhostModel, 0x0, 0x14);
OVR233_CREDITS_OBJ_ASSERT(creditGhostInst, 0x14, 0x14);
OVR233_CREDITS_OBJ_ASSERT(data_0x80_0x5, 0x28, 0x280);
OVR233_CREDITS_OBJ_ASSERT(data_0x18_0x5, 0x2a8, 0x78);
OVR233_CREDITS_OBJ_ASSERT(countdown, 0x320, 0x2);
OVR233_CREDITS_OBJ_ASSERT(unk, 0x324, 0x4);
OVR233_CREDITS_OBJ_ASSERT(creditDanceInst, 0x328, 0x4);
OVR233_CREDITS_OBJ_ASSERT(credits_posY, 0x32c, 0x2);
OVR233_CREDITS_OBJ_ASSERT(credits_topString, 0x330, 0x4);
OVR233_CREDITS_OBJ_ASSERT(epilogue_topString, 0x334, 0x4);
OVR233_CREDITS_OBJ_ASSERT(epilogue_nextString, 0x338, 0x4);
OVR233_CREDITS_OBJ_ASSERT(epilogueCount200, 0x33c, 0x2);
OVR233_CREDITS_OBJ_ASSERT(epiloguePosX, 0x33e, 0x2);
_Static_assert(sizeof(struct CreditsObj) == 0x340);

#undef OVR233_CREDITS_OBJ_ASSERT

#define OVR233_CREDITS_BSS_ASSERT(ELEMENT, OFFSET, SIZE)                      \
	_Static_assert(OFFSETOF(struct Ovr233_Credits_BSS, ELEMENT) == (OFFSET)); \
	_Static_assert(sizeof(((struct Ovr233_Credits_BSS *)0)->ELEMENT) == (SIZE))

OVR233_CREDITS_BSS_ASSERT(creditGhost_Pos, 0x0, 0x8);
OVR233_CREDITS_BSS_ASSERT(unused_Pos, 0x8, 0x8);
OVR233_CREDITS_BSS_ASSERT(creditText_PosX, 0x10, 0x4);
OVR233_CREDITS_BSS_ASSERT(CreditThread, 0x14, 0x4);
OVR233_CREDITS_BSS_ASSERT(DancerThread, 0x18, 0x4);
OVR233_CREDITS_BSS_ASSERT(dancerInst_invisible, 0x1c, 0x4);
OVR233_CREDITS_BSS_ASSERT(numStrings, 0x20, 0x2);
OVR233_CREDITS_BSS_ASSERT(ptrStrings, 0x24, 0x4);
OVR233_CREDITS_BSS_ASSERT(boolAllBlue, 0x28, 0x2);
OVR233_CREDITS_BSS_ASSERT(unused, 0x2c, 0x8);
OVR233_CREDITS_BSS_ASSERT(creditsObj, 0x34, 0x340);
_Static_assert(sizeof(struct Ovr233_Credits_BSS) == 0x374);

#undef OVR233_CREDITS_BSS_ASSERT

#ifndef CTR_NATIVE
_Static_assert(OFFSETOF(struct Ovr233_Credits_BSS, numStrings) == 0x20);
_Static_assert(OFFSETOF(struct Ovr233_Credits_BSS, ptrStrings) == 0x24);
_Static_assert(OFFSETOF(struct Ovr233_Credits_BSS, boolAllBlue) == 0x28);
_Static_assert(OFFSETOF(struct Ovr233_Credits_BSS, creditsObj) == 0x34);
#endif

extern struct Ovr233_Credits_BSS creditsBSS;
