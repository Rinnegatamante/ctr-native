#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002a3a8-0x8002a3d4
void cseq_opcode06(struct SongSeq *seq)
{
	u8 *note = seq->currNote;
	seq->vol_Curr = note[1];
	cseq_opcode_from06and07(seq);
}
