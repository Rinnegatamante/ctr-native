#ifndef CTR_SCRATCHPAD_H
#define CTR_SCRATCHPAD_H

#ifdef __vita__
#define CTR_SCRATCHPAD_ADDR                 0x98000000u
#else
#define CTR_SCRATCHPAD_ADDR                 0x1f800000u
#endif
#define CTR_SCRATCHPAD_SIZE                 0x400u
#define CTR_SCRATCHPAD_MAP_SIZE             0x1000u

// NOTE(aalhendi): Use these helpers when porting retail scratchpad temporaries.
// On PS1 this is hardware scratchpad RAM. On ctr-native, Platform_InitScratchpad
// maps an OS page at the same virtual address so raw retail addresses and
// helper-based accesses share storage. The retail scratchpad is 1 KiB; native
// maps one 4 KiB page because that is the usual minimum virtual-memory unit.
#define CTR_SCRATCHPAD_BASE                 ((u8 *)CTR_SCRATCHPAD_ADDR)
#define CTR_SCRATCHPAD_PTR(type, offset)    ((type *)(void *)(CTR_SCRATCHPAD_BASE + (offset)))
#define CTR_SCRATCHPAD_ADDR_PTR(type, addr) ((type *)(void *)(addr))

#endif
