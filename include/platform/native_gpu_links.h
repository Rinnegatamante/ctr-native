#ifndef NATIVE_GPU_LINKS_H
#define NATIVE_GPU_LINKS_H

#include <stddef.h>
#include <stdint.h>

#define NATIVE_GPU_LINK_TERMINATOR 0x00ffffffu

// NOTE(aalhendi): Native keeps PS1 GPU packet tags retail-shaped:
// [len:8][link:24]. Under CTR_NATIVE the 24-bit link is a token into
// registered host ranges, not a truncated host pointer.
void NativeGpuLinks_Reset(void);
uint32_t NativeGpuLinks_FromHostPointer(const void *hostPtr);
void *NativeGpuLinks_ToHostPointer(uint32_t token);
int NativeGpuLinks_IsTerminator(uint32_t token);
int NativeGpuLinks_IsRegisteredHostPointer(const void *hostPtr);
int NativeGpuLinks_IsRegisteredHostRange(const void *hostPtr, size_t size);
int NativeGpuLinks_RegisterRange(const void *hostStart, size_t size, uint32_t *tokenStartOut);
void NativeGpuLinks_RegisterRangeChecked(const char *label, const void *hostStart, size_t size);

#endif
