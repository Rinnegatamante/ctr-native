#if 0
// this is a type in libsnd.h
struct SndVolume
{
    u16 left;
    u16 right;
};
#endif

#ifndef CTR_NATIVE
// from TOMB5, not from psyq
// https://github.com/TOMB5/TOMB5/blob/master/EMULATOR/LIBSPU.H
typedef struct
{
	u32 mask;
	s32 mode;

	// SpuVolume from psn00b headers
	SpuVolume depth; /* reverb depth */

	s32 delay;    /* Delay Time  (ECHO, DELAY only)   */
	s32 feedback; /* Feedback    (ECHO only)          */
} SpuReverbAttr;
#endif

// similar to SndRegisterAttr in psyq libsnd.h
struct ChannelAttr
{
	// 0x0
	void *spuStartAddr;

	// as + dr = ASDR (envelope standard)

	// 0x4
	s16 ad;
	s16 sr;

	// 0x8
	s16 pitch;

	// 0xa
	s16 reverb;

	// 0xc
	s16 audioL;

	// 0xe
	s16 audioR;

	// 0x10 bytes large
};

// similar to SndVoiceStats in psyq libsnd.h
struct ChannelStats
{
	// 0x0
	struct ChannelStats *next;

	// 0x4
	struct ChannelStats *prev;

	// 0x8
	u8 flags;

	// 0x9
	char channelID;

	// 0xa
	// ??? set in "noteon"
	char unk1;

	// 0xb
	// Type (0=engineFX,1=otherFX,2=music)
	char type;

	// 0xc
	// ??? set in "noteon"
	char unk2;

	// 0xd
	// union, either or
	char drumIndex_pitchIndex;

	// 0xe
	char echo;

	// 0xf
	u8 vol;

	// 0x10
	u8 distort;

	// 0x11
	u8 LR;

	// 0x12
	s16 ad;
	s16 sr;

	// 0x16
	s16 timeLeft;

	// 0x18
	// bitshifted top 2 bytes are "CountSounds"
	int soundID; // from Sound_Play

	// 0x1c
	int startFrame;
};

enum GarageSoundPos
{
	GSP_CENTER = 0,
	GSP_LEFT,
	GSP_RIGHT,
	GSP_GONE,
	GSP_NUM
};

struct GarageFX
{
	// enum GarageSoundPos
	char gsp_curr;
	char gsp_prev;

	// 0x2
	s16 volume;

	// 0x4
	int LR;

	// 0x8
	void *audioPtr;

	// 0xC - size of each member
};

struct OtherFX
{
	// 0x0
	u8 flags;
	u8 volume;

	// 0x2
	u16 pitch;

	// 0x4
	u16 spuIndex;

	// 0x6
	u16 duration;

	// 0x8 -- size
};

struct EngineFX
{
	// 0x0
	u8 flags;
	u8 volume;

	// 0x2
	u16 pitch;

	// 0x4
	u16 unk;

	// 0x6
	u16 spuIndex;

	// 0x8 -- size
};

struct HowlHeader
{
	int magic;
	int version;
	int unk1;
	int unk2;

	// 0x10
	int numSpuAddrs;
	int numOtherFX;
	int numEngineFX;
	int numBanks;

	// 0x20
	int numSequences;
	int headerSize;

	// 0x28 -- size
};

// Start of a Cseq Pack,
// contains CseqHeader,
// then SampleInstrument array,
// then ShortSamples array,
// then songs (CseqSongHeader + Seq/Note array)
struct CseqHeader
{
	int songSize;

	// 0x4
	u8 numLongSamples;
	u8 numShortSamples;
	u16 numSongs;
};

struct SampleInstrument
{
	// 0x0
	char alwaysOne;
	u8 volume;

	// 0x2
	s16 alwaysZero;

	// 0x4
	// middle C at frequency 60
	s16 basePitch;

	// 0x6
	s16 spuIndex;

	// 0x8
	s16 ad;
	s16 sr;

	// 0xC -- end of struct
};

struct SampleDrums
{
	// 0x0
	char alwaysOne;
	u8 volume;

	// 0x2
	s16 pitch;

	// 0x4
	s16 spuIndex;

	// 0x6
	s16 alwaysZero;

	// 0x8 -- end of struct
};

// inside CseqPack (after CseqHeader, Instrument Array, and Drums Array)
struct CseqSongHeader
{
	// 0x0
	char unk;
	char numSeqs;

	// 0x2
	s16 bpm; // beats per minute

	// 0x4
	s16 tpqn; // ticks per quarter note

	// size of numSeqs
	// each seq is an array of SongNote
	// s16 seqOffsetArr[0];
};
#define SONGHEADER_GETSEQOFFARR(x) ((u32)x + sizeof(struct CseqSongHeader))

// right before first note
struct SongNoteHeader
{
	// instrument or drums
	char flags;

	char unk;

	// char notes[0];
};
#define NOTEHEADER_GETNOTES(x) ((u32)x + sizeof(struct SongNoteHeader))

#if 0
// AKA: SongNote
struct SongOpcode
{
	// 0x0
	// opcodeID (0x0 - 0xA)
	
	// 0x1
	//		opcode01: pitchIndex_drumIndex
	//		opcode05: pitchIndex_drumIndex
	//		opcode06: volume
	//		opcode07: distort
	//		opcode08: reverb
	//		opcode09: instrumentID
	//		opcode0a: distortion
	
	// 0x2
	// 		opcode05: volume
	
	// size -- unk
};
#endif

struct SongSeq
{
	// pointer in SongPool->CseqSequences
	// stored in global array 800902cc songSeq[NUM_SFX_CHANNELS]

	// 0x0
	// & 1 - playing
	// & 2 - song loops
	// & 4 - instrument or drums
	// & 8 - restart song
	char flags;

	// 0x1
	u8 soundID;
	char unk;

	// 0x3 (SampleInstrument*)
	char instrumentID;

	// 0x4
	char reverb;

	// one is curr, one is desired

	// 0x5
	u8 vol_Curr;

	// 0x6
	u8 vol_New;

	// 0x7
	u8 vol_StepRate;

	// one is curr, one is desired

	// 0x8
	u8 distort;

	// 0x9
	u8 LR;

	// 0xA
	char unk0A;

	// 0xb
	char songPoolIndex;

	// 0xc (time until next note is played)
	int NoteLength;

	// 0x10
	int NoteTimeElapsed;

	// 0x14
	// SongOpcode
	char *firstNote;

	// 0x18
	// SongOpcode
	char *currNote;

	// 0x1C -- size
};

// 80095D84
// Song in SongPool,
// not a song in HOWL file, need renaming
struct Song
{
	// 0x0
	// & 1 = Playing
	// & 2 = Paused (can be &3 in menus)
	// & 4 = needs to stop
	u8 flags;

	// 0x1
	char songPoolIndex;

	// 0x2
	// songID out of all songs in RAM
	u16 id;

	// 0x4
	int songSetActiveBits;

	// 0x8
	// ticks per quarter note
	s16 tpqn;

	// 0xA
	// beats per minute
	s16 bpm;

	// 0xC
	int tempo;

	// 0x10
	int unk10;

	// 0x14
	int timeSpentPlaying;

	// 0x18 = vol_Curr
	u8 vol_Curr;
	u8 vol_New;
	u8 vol_StepRate;

	// 0x1b
	char numSequences;

	// 0x1c array of all cseq sequences in song
	struct SongSeq *CseqSequences[0x18];
};

struct SongSet
{
	int numSeqs;
	char *ptrSongSetBits;
};

struct SampleBlockHeader
{
	s16 numSamples;

	// s16 spuIndexArr[0];
};
#define SBHEADER_GETARR(x) (s16 *)((u32)x + sizeof(struct SampleBlockHeader))

struct SpuAddrEntry
{
	u16 spuAddr;
	u16 spuSize;
};

struct Bank
{
	// 0x0
	s16 bankID;

	// 0x2
	u16 flags;

	// min and max are ranges used in FUN_80029730,
	// range for what kind of data?

	// could also be offset 0x4 as base index,
	// and offset 0x6 as number of elements

	// 0x4
	u16 min;

	// 0x6
	u16 max;

	// 8 elements of 8-byte struct
};

#if 0
enum VoiceType_SFX
{
	VT_Blasted = 1,
	VT_Spinout = 3,
	VT_Jump = 7,
	// 0xa,
	// 0xb,
	// 0xf,
	VT_Turbo = 0x10,
	VT_Crate = 0x13,
}

// This is XA_Game, move to CDSYS, and the WRONG ORDER
enum VoiceType_XAGAME2
{
	VT_ActiveTaunt1=0,
	VT_ActiveTaunt2=1,
	VT_BigAir1,
	VT_BigAir2,
	VT_Finish1,
	VT_Finish2,
	VT_Finish3,
	VT_LaughTaunt1,
	VT_LaughTaunt2,
	VT_Ouch1,
	VT_Ouch2,
	VT_PassiveTaunt1,
	VT_PassiveTaunt2,
	VT_SneakyTaunt1,
	VT_SneakyTaunt2,
	VT_Spinout1,
	VT_Spinout2,
	VT_Yay1,
	VT_Yay2,
	VT_NUM // 0x13
}
#endif

_Static_assert(sizeof(SpuReverbAttr) == 0x14);
_Static_assert(sizeof(struct ChannelAttr) == 0x10);
_Static_assert(sizeof(struct ChannelStats) == 0x20);
_Static_assert(sizeof(struct SongSeq) == 0x1C);
_Static_assert(sizeof(struct Song) == 0x7C);
