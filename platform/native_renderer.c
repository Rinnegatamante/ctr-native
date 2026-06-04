#include "platform/native_renderer.h"
#include "platform/native_renderer_backend.h"

int NativeRenderer_InitialiseRender(char *windowName, int width, int height, int fullscreen)
{
	return NativeRendererBackend_InitialiseRender(windowName, width, height, fullscreen);
}

int NativeRenderer_InitialisePSX(void)
{
	return NativeRendererBackend_InitialisePSX();
}

void NativeRenderer_Shutdown(void)
{
	NativeRendererBackend_Shutdown();
}

void NativeRenderer_ResetDevice(void)
{
	NativeRendererBackend_ResetDevice();
}

void NativeRenderer_BeginScene(void)
{
	NativeRendererBackend_BeginScene();
}

void NativeRenderer_EndScene(void)
{
	NativeRendererBackend_EndScene();
}

void NativeRenderer_UpdateSwapIntervalState(int swapInterval)
{
	NativeRendererBackend_UpdateSwapIntervalState(swapInterval);
}

void NativeRenderer_SwapWindow(void)
{
	NativeRendererBackend_SwapWindow();
}

void NativeRenderer_StoreFrameBuffer(int x, int y, int w, int h)
{
	NativeRendererBackend_StoreFrameBuffer(x, y, w, h);
}

void NativeRenderer_PresentVRAMDisplay(void)
{
	NativeRendererBackend_PresentVRAMDisplay();
}

void NativeRenderer_SaveVRAM(const char *outputFileName, int x, int y, int width, int height, int readFromFramebuffer)
{
	NativeRendererBackend_SaveVRAM(outputFileName, x, y, width, height, readFromFramebuffer);
}

void NativeRenderer_Clear(int x, int y, int w, int h, u8 r, u8 g, u8 b)
{
	NativeRendererBackend_Clear(x, y, w, h, r, g, b);
}

void NativeRenderer_ClearVRAM(int x, int y, int w, int h, u8 r, u8 g, u8 b)
{
	NativeRendererBackend_ClearVRAM(x, y, w, h, r, g, b);
}

void NativeRenderer_CopyVRAM(u16 *src, int x, int y, int w, int h, int dstX, int dstY)
{
	NativeRendererBackend_CopyVRAM(src, x, y, w, h, dstX, dstY);
}

void NativeRenderer_ReadVRAM(u16 *dst, int x, int y, int dstW, int dstH)
{
	NativeRendererBackend_ReadVRAM(dst, x, y, dstW, dstH);
}

void NativeRenderer_UpdateVRAM(void)
{
	NativeRendererBackend_UpdateVRAM();
}

void NativeRenderer_ReadFramebufferDataToVRAM(void)
{
	NativeRendererBackend_ReadFramebufferDataToVRAM();
}

TextureID NativeRenderer_GetVRAMTexture(void)
{
	return NativeRendererBackend_GetVRAMTexture();
}

TextureID NativeRenderer_GetWhiteTexture(void)
{
	return NativeRendererBackend_GetWhiteTexture();
}

void NativeRenderer_SetBlendMode(BlendMode blendMode)
{
	NativeRendererBackend_SetBlendMode(blendMode);
}

void NativeRenderer_SetStencilMode(int drawPrim)
{
	NativeRendererBackend_SetStencilMode(drawPrim);
}

void NativeRenderer_SetOffscreenState(const RECT16 *offscreenRect, int enable)
{
	NativeRendererBackend_SetOffscreenState(offscreenRect, enable);
}

void NativeRenderer_SetupClipMode(const RECT16 *clipRect, int enable)
{
	NativeRendererBackend_SetupClipMode(clipRect, enable);
}

void NativeRenderer_SetTexture(TextureID texture, TexFormat texFormat)
{
	NativeRendererBackend_SetTexture(texture, texFormat);
}

void NativeRenderer_SetOverrideTextureSize(int width, int height)
{
	NativeRendererBackend_SetOverrideTextureSize(width, height);
}

void NativeRenderer_SetPSXTextureSemiTransPass(int pass)
{
	NativeRendererBackend_SetPSXTextureSemiTransPass(pass);
}

void NativeRenderer_SetPSXDrawMaskSet(int maskSet)
{
	NativeRendererBackend_SetPSXDrawMaskSet(maskSet);
}

void NativeRenderer_UpdateVertexBuffer(const GrVertex *vertices, int count)
{
	NativeRendererBackend_UpdateVertexBuffer(vertices, count);
}

void NativeRenderer_DrawTriangles(int startVertex, int triangles)
{
	NativeRendererBackend_DrawTriangles(startVertex, triangles);
}

void NativeRenderer_PushDebugLabel(const char *label)
{
	NativeRendererBackend_PushDebugLabel(label);
}

void NativeRenderer_PopDebugLabel(void)
{
	NativeRendererBackend_PopDebugLabel();
}
