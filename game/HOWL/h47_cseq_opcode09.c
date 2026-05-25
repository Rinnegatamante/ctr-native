#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002a494-0x8002a4a8
void cseq_opcode09(struct SongSeq *seq)
{
	u8 *currNote = seq->currNote;
	seq->instrumentID = currNote[1];
}
