
struct MaskHint
{
	struct MaskHint *self; // why?
	s16 scale;

	// end of struct
};

struct BossGarageDoor
{
	int direction; // 1, 0, -1

	// so you can't spam open/close
	int cooldown;

	struct Instance *garageTopInst;

	s16 rot[4];

	// 0x14 bytes large
};

#if 0
struct AdvPause {

};
#endif

enum WoodDoorCamFlags
{
	WdCam_FlyingOut = 1,
	WdCam_FullyOut = 2, // unused
	WdCam_FlyingIn = 4, // unused
	WdCam_CutscenePlaying = 0x10
};

struct WoodDoor
{
	struct Instance *otherDoor;
	struct Instance *keyInst[4];

	// 0x14 (5)
	s16 doorRot[4];

	// 0x1c (7)
	s16 camFlags;
	s16 camTimer_unused;

	// 0x20 (8)
	int hudFlags;

	// 0x24 (9)
	s16 frameCount_unused;
	s16 frameCount_doorOpenAnim;

	// 0x28 (10)
	s16 keyRot[4];

	// 0x30 (12)
	s16 keyOrbit;
	s16 keyShrinkFrame;

	// 0x34
	int doorID;

	// 0x38 bytes large
};

enum WarpPadInstanceSet
{
	// instances that appear
	// when warppad is closed
	WPIS_CLOSED_ITEM = 0,
	WPIS_CLOSED_X,
	WPIS_CLOSED_1S,
	WPIS_CLOSED_10S,

	// istances that appear
	// when warppad is open
	WPIS_OPEN_BEAM,
	WPIS_OPEN_RING1,
	WPIS_OPEN_RING2,
	WPIS_OPEN_PRIZE1,
	WPIS_OPEN_PRIZE2,
	WPIS_OPEN_PRIZE3,

	WPIS_NUM_INSTANCES
};

struct WarpPad
{
	// 0x0
	struct Instance *inst[WPIS_NUM_INSTANCES];

	// 0x28
	s16 spinRot_Prize[4];

	// 0x30
	s16 spinRot_Wisp[2][4];

	// 0x40
	s16 spinRot_Beam[4];

	// 0x48
	s16 spinRot_Rewards[4];

	// 0x50
	s16 specLightGem[4];

	// 0x58
	s16 specLightRelic[4];

	// 0x60
	s16 specLightToken[4];

	// 0x68
	s16 digit10s;

	// 0x6a
	s16 digit1s;

	// 0x6c (1b*4)
	s16 levelID;

	// 0x6e
	// 0/3    1/3     2/3
	// 0x0    0x555   0xAAA
	u16 thirds[3];

	// 0x74
	s16 boolEnteredWarppad;

	// 0x76
	s16 framesWarping;

	// 0x78 -- size
};

struct SaveObj
{
	// 0x0
	struct Instance *inst;
	// 0x4
	u16 flags;
	// 0x6
	s16 scanlineFrame;
	// 0x8
	u8 hudFlagBackup;

	// total size unk
};

struct OverlayRDATA_232
{
	// 0x800aba3c
	s16 battleTrackArr[8];

	// 0x800aba4c
	s16 bossTracks[6];

	// 0x800aba58
	s16 bossIDs[6];

	// 0x800aba64
	char s_garage[8];
	char s_garagetop[0xc];
	char s_saveobj[8];
	char s_scan[8];
	char s_key[4];

	// 0x800aba8c
	s16 keyFrame[0xc];
};

struct OverlayDATA_232
{
	// 800b4ddc (3*5 plus padding)
	s16 specLightGem[16];

	// 800b4dfc (3*5 plus padding)
	s16 specLightRelic[16];

	// 800b4e1c (3*5 plus padding)
	s16 specLightToken[16];

	// 800b4e3c
	struct MenuRow rowsTokenRelic[3];

	// 800b4e50
	struct RectMenu menuTokenRelic;

	// 800b4e7c
	s16 arrKeysNeeded[5];

	// 800b4e86
	s16 levelID;

	// 800b4e88
	int timeCrystalChallenge[7];

	// 800b4ea4
	s16 saveObjCameraOffset[4];

	// 800b4eac
	s16 primOffsetXY_LoadSave[5 * 2];

	// 800b4ec0
	s16 primOffsetXY_HubArrow[5 * 2];

	struct HubItem
	{
		// 0x0
		s16 posX;
		s16 posY;

		// 0x4
		s16 angle;

		// 0x6
		// 0x03: boss
		// 0x04: warppad
		// 0x64: saveload
		// -1: (1 key) Arrow beach->gemstone
		// -2: (0 key) Arrow gemstone->beach
		// -3: (0 key) Arrow gemstone->ruins
		// -4: (2 key) Arrow beach->glacier
		// -5: (3 key) Arrow glacier->citadel
		s16 iconType;

		// 0x8 -- size
	}
	// 800b4ed4
	// 2 arrows, boss, save/load, null(0xFFFF)
	hubItems_hub1[5],
	    // 800b4efc
	    hubItems_hub2[5],
	    // 800b4f24
	    hubItems_hub3[5],
	    // 800b4f4c (3 arrows)
	    hubItems_hub4[6],
	    // 800b4f7c (1 arrow)
	    hubItems_hub5[4];

	// 800b4f9c -- array of pointers:
	//		800b4ed4 800b4efc 800b4f24
	//		800b4f4c 800b4f7c
	s16 *hubItemsXY_ptrArray[5];

	// 800b4fb0
	s16 hubArrowXY_Inner[2 * 3];

	// 800b4fbc
	s16 hubArrowXY_Outter[2 * 4];

	// 800b4fcc
	s16 loadSave_pos[2 * 4];

	// 800b4fdc
	int loadSave_col[4]; // maybe should be `char*` instead of `int`

	// 800b4fec
	s16 hubArrow_pos[2 * 3];

	// 800B4FF8
	int hubArrow_col1[3]; // maybe should be `char*` instead of `int`

	// 800b5004
	int hubArrow_col2[3];

	// 800b5010
	int hubArrowGray1[3];
	int hubArrowGray2[3];

	// 800b5028
	// 8 bytes each
	struct
	{
		// can be -1 if not hub page
		s16 hubID;

		// can be -1 for hubs, which then
		// get name from MetaDataLev
		s16 titleLng;

		// 0: draw tracks
		// 1: draw 5 tokens
		// 2: draw relics
		s16 type;

		s16 characterID_Boss;
	} advPausePages[7];

	// 0x800B5060
	// 0,1,2,3,4: Gems
	// 5: Key (for all pages)
	// 6,7,8: 3-Relic page
	// 9,10,11,12,13: Tokens
	// 14: Trophy
	struct
	{
		// 0x0
		s16 modelID;
		s16 scale;

		// 0x4
		int color;

		// 0x8
		// same for all gems
		int instFlags;

		// 0xC
		// same for all gems
		s16 specLight[4];


		// 0x14 bytes each
	} advPauseInst[15];

	// 0x800B518C
	struct RectMenu menuHintMenu;

	// 0x800B51B8
	s16 fiveArrow_pos[2 * 3];

	// 0x800b51c4
	int fiveArrow_col1[3];

	// 0x800b51d0
	int fiveArrow_col2[3];

	// 0x800b51dc
	s16 maskPos[4];

	// 0x800b51e4
	s16 maskRot[4];

	// 0x800b51ec
	s16 maskScale;

	// 0x800b51ee
	s16 maskCooldown;

	// 0x800b51f0
	s16 maskOffsetPos[4];

	// 0x800b51f8
	s16 maskOffsetRot[4];

	// 0x800b5200
	s16 maskVars[12];

	// 0x800b5218
	int maskFrameCurr;

	// 0x800b521c
	struct ParticleEmitter emSet_maskSpawn[0xA];

	// 0x800b5384
	struct ParticleEmitter emSet_maskLeave[0xA];

	// 0x800b54ec
	s16 maskAudioSettings[4];

	// 800b54f4
	// 20 hints, last two entries are null
	s16 hintMenu_lngIndexArr[22];

	// 800b5520
	s16 eyePos[4];

	// 800b5528
	s16 lookAtPos[4];

	// 800b5530
	int colorQuad[4]; // maybe should be `char*` instead of `int`

	// 800b5540
	int colorTri[3]; // maybe should be `char*` instead of `int`

	// 800b554c
	s16 pausePageDir;

	// 800b554e
	s16 pausePageTimer;

	// 800b5550
	s16 pausePagePrev;

	// 800b5552
	s16 pausePageCurr;

	// 800b5554
	s16 pausePageDir_dup;
	s16 padding3;

	// 800b5558
	s16 maskHintID;
	s16 padding_maskHintID;

	// 800b555c
	int maskAngle;

	// 800b5560
	s16 maskCamPosStart[4];

	// 800b5568
	s16 maskCamRotStart[4];

	// 800b5570
	int maskWarppadDelayFrames;

	// 800b5574
	s16 maskWarppadBoolInterrupt;
	s16 padding_maskWarppadBoolInterrupt;

	// 800b5578
	struct PauseObject *ptrPauseObject;

	struct PauseObject
	{
		// 0x0
		struct
		{
			// 0x0
			s16 indexAdvPauseInst;
			s16 unlockFlag;

			// 0x4
			s16 rot[4];

			// 0xC
			struct Instance *inst;

			// 0x10 -- size
		} PauseMember[0xe];

		// 0xE0
		struct Thread *t;

		// 0xe4 -- size
	} pauseObject; // 800b557c

	// 800B5660
	int hintMenu_boolViewHint;

	// 800B5664
	int hintMenu_scrollIndex;

	// 800B5668
	char audioBackup[4];

	// 800B566c
	s16 maskSpawnFrame;
	s16 padding4;

	// 800b5670
	int unkModeHubItems;
};

extern struct OverlayRDATA_232 R232;
extern struct OverlayDATA_232 D232;
