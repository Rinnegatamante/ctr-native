#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002a3d4-0x8002a400
void cseq_opcode07(struct SongSeq *seq)
{
	u8 *note = seq->currNote;
	seq->LR = note[1];
	cseq_opcode_from06and07(seq);
}
