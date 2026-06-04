/*
 * Derived from REDRIVER2/PsyCross MIT source:
 * externals/PsyCross/src/gpu/PsyX_GPU.cpp
 * See THIRD_PARTY_NOTICES.md for copyright and license details.
 */

#include "platform/native_gpu.h"

#include <SDL3/SDL.h>

#include "PsyX/PsyX_public.h"
#include "PsyX/PsyX_globals.h"
#include "platform/native_log.h"
#include "platform/native_renderer.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void Platform_PollHostEvents(void);

#define NATIVE_GPU_LOG(fmt, ...)   Platform_Log("[CTR GPU] " fmt, ##__VA_ARGS__)
#define NATIVE_GPU_ERROR(fmt, ...) Platform_LogError("[CTR GPU] [%s] - " fmt, __func__, ##__VA_ARGS__)

#define GET_TPAGE_BLEND(tpage)     ((BlendMode)(((tpage >> 5) & 3) + 1))

#define GET_TPAGE_DITHER(tpage)    ((tpage >> 9) & 0x1)

#define GET_CLUT_X(clut)           ((clut & 0x3F) << 4)
#define GET_CLUT_Y(clut)           (clut >> 6)

static TexFormat GetTPageFormat(int tpage)
{
	const int mode = (tpage >> 7) & 0x3;

	// NOTE(aalhendi): ctr-native local divergence from upstream PsyCross. PS1
	// mode 3 is reserved; TF_32_BIT_RGBA is only for explicit native override
	// textures, not raw retail TPAGE mode bits.
	return mode == 3 ? TF_16_BIT : (TexFormat)mode;
}

static short GetTPageBase(int tpage)
{
	const u16 page = (u16)tpage;

	// NOTE(aalhendi): ctr-native local divergence for CTR retail emitters. The
	// shader wants xPage + yPage * 16, not raw draw-mode bits.
	return (s16)((page & 0xf) | ((page & 0x10) ? 0x10 : 0));
}

DISPENV activeDispEnv;
DRAWENV activeDrawEnv;

static const char *currentSplitDebugText = NULL;
TextureID overrideTexture = 0;
int overrideTextureWidth = 0;
int overrideTextureHeight = 0;

int g_GPUDisabledState = 0;
int g_DrawPrimMode = 0;
static bool g_psxDrawMaskSet = false;

typedef struct
{
	DRAWENV drawenv;
	DISPENV dispenv;

	BlendMode blendMode;

	TexFormat texFormat;
	TextureID textureId;

	int drawPrimMode;
	bool psxTexturedSemiTrans;
	bool psxDrawMaskSet;

	u16 startVertex;
	u16 numVerts;

	const char *debugText;
} GPUDrawSplit;

#define MAX_DRAW_SPLITS 4096

GrVertex g_vertexBuffer[MAX_VERTEX_BUFFER_SIZE];
GPUDrawSplit g_splits[MAX_DRAW_SPLITS];

int g_vertexIndex = 0;
int g_splitIndex = 0;

void ClearSplits()
{
	currentSplitDebugText = NULL;
	g_vertexIndex = 0;
	g_splitIndex = 0;
	g_splits[0].texFormat = (TexFormat)0xFFFF;
	g_splits[0].psxTexturedSemiTrans = false;
	g_splits[0].psxDrawMaskSet = false;
}

static void DrawEnvDimensionsInt(int *width, int *height)
{
	if (activeDrawEnv.dfe)
	{
		*width = activeDispEnv.disp.w;
		*height = activeDispEnv.disp.h;
	}
	else
	{
		*width = activeDrawEnv.clip.w;
		*height = activeDrawEnv.clip.h;
	}
}

static void DrawEnvDimensionsFloat(float *width, float *height)
{
	int intWidth;
	int intHeight;

	DrawEnvDimensionsInt(&intWidth, &intHeight);
	*width = (float)intWidth;
	*height = (float)intHeight;
}

void DrawEnvOffset(float *ofsX, float *ofsY)
{
	if (activeDrawEnv.dfe)
	{
		int w, h;
		DrawEnvDimensionsInt(&w, &h);

		if (w <= 0)
			w = 1;

		// NOTE(aalhendi): Convert PS1 VRAM-page draw offsets into display-relative host-screen offsets.
		// CTR alternates draw pages at y=0 and y=0x128; using raw modulo VRAM offsets shifts every other native frame vertically.
		*ofsX = activeDrawEnv.ofs[0] - activeDispEnv.disp.x;
		*ofsY = activeDrawEnv.ofs[1] - activeDispEnv.disp.y;
	}
	else
	{
		*ofsX = 0.0f;
		*ofsY = 0.0f;
	}
}

void LineSwapSourceVerts(VERTTYPE **p0, VERTTYPE **p1, u8 **c0, u8 **c1)
{
	// swap line coordinates for left-to-right and up-to-bottom direction
	if (((*p0)[0] > (*p1)[0]) || ((*p0)[1] > (*p1)[1] && (*p0)[0] == (*p1)[0]))
	{
		VERTTYPE *tmp = *p0;
		*p0 = *p1;
		*p1 = tmp;

		u8 *tmpCol = *c0;
		*c0 = *c1;
		*c1 = tmpCol;
	}
}

void MakeLineArray(GrVertex *vertex, VERTTYPE *p0, VERTTYPE *p1)
{
	const VERTTYPE dx = p1[0] - p0[0];
	const VERTTYPE dy = p1[1] - p0[1];

	float ofsX, ofsY;
	DrawEnvOffset(&ofsX, &ofsY);

	memset(vertex, 0, sizeof(GrVertex) * 4);

	if (dx > abs((s16)dy))
	{ // horizontal
		vertex[0].x = p0[0] + ofsX;
		vertex[0].y = p0[1] + ofsY;

		vertex[1].x = p1[0] + ofsX + 1;
		vertex[1].y = p1[1] + ofsY;

		vertex[2].x = vertex[1].x;
		vertex[2].y = vertex[1].y + 1;

		vertex[3].x = vertex[0].x;
		vertex[3].y = vertex[0].y + 1;
	}
	else
	{ // vertical
		vertex[0].x = p0[0] + ofsX;
		vertex[0].y = p0[1] + ofsY;

		vertex[1].x = p1[0] + ofsX;
		vertex[1].y = p1[1] + ofsY + 1;

		vertex[2].x = vertex[1].x + 1;
		vertex[2].y = vertex[1].y;

		vertex[3].x = vertex[0].x + 1;
		vertex[3].y = vertex[0].y;
	} // TODO diagonal line alignment
}

void MakeVertexTriangle(GrVertex *vertex, VERTTYPE *p0, VERTTYPE *p1, VERTTYPE *p2)
{
	assert(p0);
	assert(p1);
	assert(p2);

	float ofsX, ofsY;
	DrawEnvOffset(&ofsX, &ofsY);

	memset(vertex, 0, sizeof(GrVertex) * 3);

	vertex[0].x = p0[0] + ofsX;
	vertex[0].y = p0[1] + ofsY;

	vertex[1].x = p1[0] + ofsX;
	vertex[1].y = p1[1] + ofsY;

	vertex[2].x = p2[0] + ofsX;
	vertex[2].y = p2[1] + ofsY;
}

void MakeVertexQuad(GrVertex *vertex, VERTTYPE *p0, VERTTYPE *p1, VERTTYPE *p2, VERTTYPE *p3)
{
	assert(p0);
	assert(p1);
	assert(p2);
	assert(p3);

	float ofsX, ofsY;
	DrawEnvOffset(&ofsX, &ofsY);

	memset(vertex, 0, sizeof(GrVertex) * 4);

	vertex[0].x = p0[0] + ofsX;
	vertex[0].y = p0[1] + ofsY;

	vertex[1].x = p1[0] + ofsX;
	vertex[1].y = p1[1] + ofsY;

	vertex[2].x = p2[0] + ofsX;
	vertex[2].y = p2[1] + ofsY;

	vertex[3].x = p3[0] + ofsX;
	vertex[3].y = p3[1] + ofsY;
}

void MakeVertexRect(GrVertex *vertex, VERTTYPE *p0, s16 w, s16 h)
{
	assert(p0);

	float ofsX, ofsY;
	DrawEnvOffset(&ofsX, &ofsY);

	memset(vertex, 0, sizeof(GrVertex) * 4);

	vertex[0].x = p0[0] + ofsX;
	vertex[0].y = p0[1] + ofsY;

	vertex[1].x = vertex[0].x;
	vertex[1].y = vertex[0].y + h;

	vertex[2].x = vertex[0].x + w;
	vertex[2].y = vertex[0].y + h;

	vertex[3].x = vertex[0].x + w;
	vertex[3].y = vertex[0].y;
}

void MakeTexcoordQuad(GrVertex *vertex, u8 *uv0, u8 *uv1, u8 *uv2, u8 *uv3, s16 page, s16 clut, u8 dither)
{
	assert(uv0);
	assert(uv1);
	assert(uv2);
	assert(uv3);

	const u8 bright = 2;
	const short texPage = GetTPageBase(page);

	vertex[0].u = uv0[0];
	vertex[0].v = uv0[1];
	vertex[0].bright = bright;
	vertex[0].dither = dither;
	vertex[0].page = texPage;
	vertex[0].clut = clut;

	vertex[1].u = uv1[0];
	vertex[1].v = uv1[1];
	vertex[1].bright = bright;
	vertex[1].dither = dither;
	vertex[1].page = texPage;
	vertex[1].clut = clut;

	vertex[2].u = uv2[0];
	vertex[2].v = uv2[1];
	vertex[2].bright = bright;
	vertex[2].dither = dither;
	vertex[2].page = texPage;
	vertex[2].clut = clut;

	vertex[3].u = uv3[0];
	vertex[3].v = uv3[1];
	vertex[3].bright = bright;
	vertex[3].dither = dither;
	vertex[3].page = texPage;
	vertex[3].clut = clut;
	/*
	if (g_cfg_bilinearFiltering)
	{
	    vertex[0].tcx = -1;
	    vertex[0].tcy = -1;

	    vertex[1].tcx = -1;
	    vertex[1].tcy = -1;

	    vertex[2].tcx = -1;
	    vertex[2].tcy = -1;

	    vertex[3].tcx = -1;
	    vertex[3].tcy = -1;
	}*/
}

void MakeTexcoordTriangle(GrVertex *vertex, u8 *uv0, u8 *uv1, u8 *uv2, s16 page, s16 clut, u8 dither)
{
	assert(uv0);
	assert(uv1);
	assert(uv2);

	const u8 bright = 2;
	const short texPage = GetTPageBase(page);

	vertex[0].u = uv0[0];
	vertex[0].v = uv0[1];
	vertex[0].bright = bright;
	vertex[0].dither = dither;
	vertex[0].page = texPage;
	vertex[0].clut = clut;

	vertex[1].u = uv1[0];
	vertex[1].v = uv1[1];
	vertex[1].bright = bright;
	vertex[1].dither = dither;
	vertex[1].page = texPage;
	vertex[1].clut = clut;

	vertex[2].u = uv2[0];
	vertex[2].v = uv2[1];
	vertex[2].bright = bright;
	vertex[2].dither = dither;
	vertex[2].page = texPage;
	vertex[2].clut = clut;
	/*
	if (g_cfg_bilinearFiltering)
	{
	    vertex[0].tcx = -1;
	    vertex[0].tcy = -1;

	    vertex[1].tcx = -1;
	    vertex[1].tcy = -1;

	    vertex[2].tcx = -1;
	    vertex[2].tcy = -1;

	    vertex[3].tcx = -1;
	    vertex[3].tcy = -1;
	}*/
}

void MakeTexcoordRect(GrVertex *vertex, u8 *uv, s16 page, s16 clut, s16 w, s16 h)
{
	assert(uv);

	// sim overflow
	if ((int)uv[0] + w > 255)
		w = 255 - uv[0];
	if ((int)uv[1] + h > 255)
		h = 255 - uv[1];

	const u8 bright = 2;
	const u8 dither = 0;
	const short texPage = GetTPageBase(page);

	vertex[0].u = uv[0];
	vertex[0].v = uv[1];
	vertex[0].bright = bright;
	vertex[0].dither = dither;
	vertex[0].page = texPage;
	vertex[0].clut = clut;

	vertex[1].u = uv[0];
	vertex[1].v = uv[1] + h;
	vertex[1].bright = bright;
	vertex[1].dither = dither;
	vertex[1].page = texPage;
	vertex[1].clut = clut;

	vertex[2].u = uv[0] + w;
	vertex[2].v = uv[1] + h;
	vertex[2].bright = bright;
	vertex[2].dither = dither;
	vertex[2].page = texPage;
	vertex[2].clut = clut;

	vertex[3].u = uv[0] + w;
	vertex[3].v = uv[1];
	vertex[3].bright = bright;
	vertex[3].dither = dither;
	vertex[3].page = texPage;
	vertex[3].clut = clut;

	if (g_cfg_bilinearFiltering)
	{
		vertex[0].tcx = -1;
		vertex[0].tcy = -1;

		vertex[1].tcx = -1;
		vertex[1].tcy = -1;

		vertex[2].tcx = -1;
		vertex[2].tcy = -1;

		vertex[3].tcx = -1;
		vertex[3].tcy = -1;
	}
}

void MakeTexcoordLineZero(GrVertex *vertex, u8 dither)
{
	const u8 bright = 1;

	vertex[0].u = 0;
	vertex[0].v = 0;
	vertex[0].bright = bright;
	vertex[0].dither = dither;
	vertex[0].page = 0;
	vertex[0].clut = 0;

	vertex[1].u = 0;
	vertex[1].v = 0;
	vertex[1].bright = bright;
	vertex[1].dither = dither;
	vertex[1].page = 0;
	vertex[1].clut = 0;

	vertex[2].u = 0;
	vertex[2].v = 0;
	vertex[2].bright = bright;
	vertex[2].dither = dither;
	vertex[2].page = 0;
	vertex[2].clut = 0;

	vertex[3].u = 0;
	vertex[3].v = 0;
	vertex[3].bright = bright;
	vertex[3].dither = dither;
	vertex[3].page = 0;
	vertex[3].clut = 0;
}

void MakeTexcoordTriangleZero(GrVertex *vertex, u8 dither)
{
	const u8 bright = 1;

	vertex[0].u = 0;
	vertex[0].v = 0;
	vertex[0].bright = bright;
	vertex[0].dither = dither;
	vertex[0].page = 0;
	vertex[0].clut = 0;

	vertex[1].u = 0;
	vertex[1].v = 0;
	vertex[1].bright = bright;
	vertex[1].dither = dither;
	vertex[1].page = 0;
	vertex[1].clut = 0;

	vertex[2].u = 0;
	vertex[2].v = 0;
	vertex[2].bright = bright;
	vertex[2].dither = dither;
	vertex[2].page = 0;
	vertex[2].clut = 0;
}

void MakeTexcoordQuadZero(GrVertex *vertex, u8 dither)
{
	const u8 bright = 1;

	vertex[0].u = 0;
	vertex[0].v = 0;
	vertex[0].bright = bright;
	vertex[0].dither = dither;
	vertex[0].page = 0;
	vertex[0].clut = 0;

	vertex[1].u = 0;
	vertex[1].v = 0;
	vertex[1].bright = bright;
	vertex[1].dither = dither;
	vertex[1].page = 0;
	vertex[1].clut = 0;

	vertex[2].u = 0;
	vertex[2].v = 0;
	vertex[2].bright = bright;
	vertex[2].dither = dither;
	vertex[2].page = 0;
	vertex[2].clut = 0;

	vertex[3].u = 0;
	vertex[3].v = 0;
	vertex[3].bright = bright;
	vertex[3].dither = dither;
	vertex[3].page = 0;
	vertex[3].clut = 0;
}

void MakeColourNoShade(GrVertex *vertex, int n)
{
	--n;
	while (n >= 0)
	{
		vertex[n].r = 128;
		vertex[n].g = 128;
		vertex[n].b = 128;
		vertex[n].a = 255;
		--n;
	}
}

void MakeColourLine(GrVertex *vertex, bool shadeTexOn, u8 *col0, u8 *col1)
{
	if (!shadeTexOn)
	{
		MakeColourNoShade(vertex, 4);
		return;
	}
	assert(col0);
	assert(col1);

	vertex[0].r = col0[0];
	vertex[0].g = col0[1];
	vertex[0].b = col0[2];
	vertex[0].a = 255;

	vertex[1].r = col1[0];
	vertex[1].g = col1[1];
	vertex[1].b = col1[2];
	vertex[1].a = 255;

	vertex[2].r = col1[0];
	vertex[2].g = col1[1];
	vertex[2].b = col1[2];
	vertex[2].a = 255;

	vertex[3].r = col0[0];
	vertex[3].g = col0[1];
	vertex[3].b = col0[2];
	vertex[3].a = 255;
}

void MakeColourTriangle(GrVertex *vertex, bool shadeTexOn, u8 *col0, u8 *col1, u8 *col2)
{
	if (!shadeTexOn)
	{
		MakeColourNoShade(vertex, 3);
		return;
	}

	assert(col0);
	assert(col1);
	assert(col2);

	vertex[0].r = col0[0];
	vertex[0].g = col0[1];
	vertex[0].b = col0[2];
	vertex[0].a = 255;

	vertex[1].r = col1[0];
	vertex[1].g = col1[1];
	vertex[1].b = col1[2];
	vertex[1].a = 255;

	vertex[2].r = col2[0];
	vertex[2].g = col2[1];
	vertex[2].b = col2[2];
	vertex[2].a = 255;
}

void MakeColourQuad(GrVertex *vertex, bool shadeTexOn, u8 *col0, u8 *col1, u8 *col2, u8 *col3)
{
	if (!shadeTexOn)
	{
		MakeColourNoShade(vertex, 4);
		return;
	}

	assert(col0);
	assert(col1);
	assert(col2);
	assert(col3);

	vertex[0].r = col0[0];
	vertex[0].g = col0[1];
	vertex[0].b = col0[2];
	vertex[0].a = 255;

	vertex[1].r = col1[0];
	vertex[1].g = col1[1];
	vertex[1].b = col1[2];
	vertex[1].a = 255;

	vertex[2].r = col2[0];
	vertex[2].g = col2[1];
	vertex[2].b = col2[2];
	vertex[2].a = 255;

	vertex[3].r = col3[0];
	vertex[3].g = col3[1];
	vertex[3].b = col3[2];
	vertex[3].a = 255;
}

void TriangulateQuad()
{
	/*
	Triangulate like this:

	v0--v1
	|  / |
	| /  |
	v2--v3

	NOTE: v2 swapped with v3 during primitive parsing but it not shown here
	*/

	g_vertexBuffer[g_vertexIndex + 4] = g_vertexBuffer[g_vertexIndex + 3];

	g_vertexBuffer[g_vertexIndex + 5] = g_vertexBuffer[g_vertexIndex + 2];
	g_vertexBuffer[g_vertexIndex + 2] = g_vertexBuffer[g_vertexIndex + 3];
	g_vertexBuffer[g_vertexIndex + 3] = g_vertexBuffer[g_vertexIndex + 1];
}

//------------------------------------------------------------------------------------------------------------------------

static void AddSplit(bool semiTrans, bool textured)
{
	int tpage = activeDrawEnv.tpage;
	GPUDrawSplit *curSplit = &g_splits[g_splitIndex];

	BlendMode blendMode = semiTrans ? GET_TPAGE_BLEND(tpage) : BM_NONE;
	TexFormat texFormat = GetTPageFormat(tpage);
	TextureID textureId = textured ? NativeRenderer_GetVRAMTexture() : NativeRenderer_GetWhiteTexture();
	bool psxTexturedSemiTrans = semiTrans && textured && overrideTexture == 0;

	if (textured && overrideTexture != 0)
	{
		// override texture format, zero tpage
		texFormat = TF_32_BIT_RGBA;
		textureId = overrideTexture;
		psxTexturedSemiTrans = false;
	}

	// FIXME: compare drawing environment too?
	if (!psxTexturedSemiTrans && curSplit->blendMode == blendMode && curSplit->texFormat == texFormat && curSplit->textureId == textureId &&
	    curSplit->drawPrimMode == g_DrawPrimMode && curSplit->psxTexturedSemiTrans == psxTexturedSemiTrans && curSplit->psxDrawMaskSet == g_psxDrawMaskSet &&
	    curSplit->drawenv.clip.x == activeDrawEnv.clip.x && curSplit->drawenv.clip.y == activeDrawEnv.clip.y &&
	    curSplit->drawenv.clip.w == activeDrawEnv.clip.w && curSplit->drawenv.clip.h == activeDrawEnv.clip.h && curSplit->drawenv.dfe == activeDrawEnv.dfe &&
	    curSplit->debugText == currentSplitDebugText)
	{
		return;
	}

	curSplit->numVerts = g_vertexIndex - curSplit->startVertex;

	if (g_splitIndex + 1 >= MAX_DRAW_SPLITS)
	{
		NATIVE_GPU_ERROR("MAX_DRAW_SPLITS reached (too many blend modes, texture formats, drawEnv clip rects, dfe switches), expect rendering errors\n");
		return;
	}

	GPUDrawSplit *split = &g_splits[++g_splitIndex];
	split->blendMode = blendMode;
	split->texFormat = texFormat;
	split->textureId = textureId;
	split->drawPrimMode = g_DrawPrimMode;
	split->psxTexturedSemiTrans = psxTexturedSemiTrans;
	split->psxDrawMaskSet = g_psxDrawMaskSet;
	split->drawenv = activeDrawEnv;
	split->dispenv = activeDispEnv;
	split->debugText = currentSplitDebugText;

	split->drawenv.tw.w = overrideTextureWidth;
	split->drawenv.tw.h = overrideTextureHeight;

	split->startVertex = g_vertexIndex;
	split->numVerts = 0;
}

void DrawSplit(const GPUDrawSplit *split)
{
	if (split->debugText)
		NativeRenderer_PushDebugLabel(split->debugText);

	NativeRenderer_SetStencilMode(split->drawPrimMode); // draw with mask 0x16

	NativeRenderer_SetTexture(split->textureId, split->texFormat);

	if (split->texFormat == TF_32_BIT_RGBA)
		NativeRenderer_SetOverrideTextureSize(split->drawenv.tw.w, split->drawenv.tw.h);

	NativeRenderer_SetPSXDrawMaskSet(split->psxDrawMaskSet);

	const bool drawOnScreen = split->drawenv.dfe;
	NativeRenderer_SetupClipMode(&split->drawenv.clip, drawOnScreen);
	NativeRenderer_SetOffscreenState(&split->drawenv.clip, !drawOnScreen);

	if (split->psxTexturedSemiTrans)
	{
		// NOTE(aalhendi): CTR native renderer divergence from upstream PsyCross.
		// PS1 textured ABE only blends texels whose sampled 16-bit color has STP
		// set; non-STP texels remain opaque. Native split state is per draw,
		// so draw this primitive-sized split twice with shader-side STP masks.
		NativeRenderer_SetBlendMode(BM_NONE);
		NativeRenderer_SetPSXTextureSemiTransPass(1);
		NativeRenderer_DrawTriangles(split->startVertex, split->numVerts / 3);

		NativeRenderer_SetBlendMode(split->blendMode);
		NativeRenderer_SetPSXTextureSemiTransPass(2);
		NativeRenderer_DrawTriangles(split->startVertex, split->numVerts / 3);

		NativeRenderer_SetPSXTextureSemiTransPass(0);
	}
	else
	{
		NativeRenderer_SetBlendMode(split->blendMode);
		NativeRenderer_SetPSXTextureSemiTransPass(0);
		NativeRenderer_DrawTriangles(split->startVertex, split->numVerts / 3);
	}

	if (split->debugText)
		NativeRenderer_PopDebugLabel();
}

static void SetPSXMaskState(u32 code)
{
	g_psxDrawMaskSet = (code & 1) != 0;
}

extern int g_dbg_polygonSelected;

//
// Draws all polygons after AggregatePTAG
//
void DrawAllSplits()
{
#ifdef CTR_INTERNAL
	if (g_dbg_emulatorPaused)
	{
		for (int i = 0; i < 3; i++)
		{
			GrVertex *vert = &g_vertexBuffer[g_dbg_polygonSelected + i];
			vert->r = 255;
			vert->g = 0;
			vert->b = 0;

			NATIVE_GPU_LOG("==========================================\n");
			NATIVE_GPU_LOG("POLYGON: %d\n", g_dbg_polygonSelected);
			NATIVE_GPU_LOG("X: %d Y: %d\n", vert->x, vert->y);
			NATIVE_GPU_LOG("U: %d V: %d\n", vert->u, vert->v);
			NATIVE_GPU_LOG("TP: %d CLT: %d\n", vert->page, vert->clut);

			NATIVE_GPU_LOG("==========================================\n");
		}

		Platform_PollHostEvents();
	}
#endif

	// next code ideally should be called before EndScene
	NativeRenderer_UpdateVertexBuffer(g_vertexBuffer, g_vertexIndex);

	for (int i = 1; i <= g_splitIndex; i++)
		DrawSplit(&g_splits[i]);

	ClearSplits();
}

// forward declarations
int ParsePrimitive(P_TAG *polyTag);
int ParseTaglessPrimitive(u32 *command);

void ParsePrimitivesLinkedList(u32 *p, int singlePrimitive)
{
	if (!p)
		return;

	// setup single primitive flag (needed for AddSplits)
	g_DrawPrimMode = singlePrimitive;

	if (singlePrimitive)
	{
		P_TAG *polyTag = (P_TAG *)p;
		ParsePrimitive(polyTag);

		GPUDrawSplit *lastSplit = &g_splits[g_splitIndex];
		lastSplit->numVerts = g_vertexIndex - lastSplit->startVertex;
	}
	else
	{
		// walk OT_TAG linked list
		for (uintptr_t basePacket = (uintptr_t)p;; basePacket = (uintptr_t)nextPrim(basePacket))
		{
			const int tagLength = getlen(basePacket);
			if (tagLength > 0)
			{
				if (tagLength > 32)
				{
					NATIVE_GPU_ERROR("got invalid tag length %d, code %d\n", tagLength, ((P_TAG *)basePacket)->code);
				}

				uintptr_t currentPacket = basePacket;
				const uintptr_t endPacket = basePacket + (tagLength + P_LEN) * sizeof(u32);
				int primLength = 0;
				if (currentPacket < endPacket)
				{
					primLength = ParsePrimitive((P_TAG *)currentPacket);
					currentPacket += (primLength + P_LEN) * sizeof(u32);
				}

				while (currentPacket < endPacket)
				{
					primLength = ParseTaglessPrimitive((u32 *)currentPacket);
					currentPacket += primLength * sizeof(u32);
				}

				if (currentPacket != endPacket)
				{
					NATIVE_GPU_ERROR("did not output valid primitive or ptag length is not valid (diff=%d)\n", endPacket - currentPacket);
				}
			}

			GPUDrawSplit *lastSplit = &g_splits[g_splitIndex];
			lastSplit->numVerts = g_vertexIndex - lastSplit->startVertex;

			if (isendprim(basePacket))
				break;
		}
	}
}

static inline int IsNull(POLY_FT3 *poly)
{
	return poly->x0 == -1 && poly->y0 == -1 && poly->x1 == -1 && poly->y1 == -1 && poly->x2 == -1 && poly->y2 == -1;
}

static int ProcessFlatLines(P_TAG *polyTag)
{
	const bool shadeTexOn = true;
	const bool semiTrans = (polyTag->code & 2);
	const int primSubType = polyTag->code & 0x0C;

	switch (primSubType)
	{
	case 0x0:
	{
		LINE_F2 *poly = (LINE_F2 *)polyTag;

		AddSplit(semiTrans, false);

		VERTTYPE *p0 = &poly->x0;
		VERTTYPE *p1 = &poly->x1;
		u8 *c0 = &poly->r0;
		u8 *c1 = c0;

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		LineSwapSourceVerts(&p0, &p1, &c0, &c1);
		MakeLineArray(firstVertex, p0, p1);
		MakeTexcoordLineZero(firstVertex, 0);
		MakeColourLine(firstVertex, shadeTexOn, c0, c1);

		TriangulateQuad();

		g_vertexIndex += 6;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 3;
	}
	case 0x8: // TODO (unused)
	{
		LINE_F3 *poly = (LINE_F3 *)polyTag;

		AddSplit(semiTrans, false);

		{
			VERTTYPE *p0 = &poly->x0;
			VERTTYPE *p1 = &poly->x1;
			u8 *c0 = &poly->r0;
			u8 *c1 = c0;

			GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
			LineSwapSourceVerts(&p0, &p1, &c0, &c1);
			MakeLineArray(firstVertex, p0, p1);
			MakeTexcoordLineZero(firstVertex, 0);
			MakeColourLine(firstVertex, shadeTexOn, c0, c1);

			TriangulateQuad();

			g_vertexIndex += 6;
#if defined(DEBUG_POLY_COUNT)
			polygon_count++;
#endif
		}

		{
			VERTTYPE *p0 = &poly->x1;
			VERTTYPE *p1 = &poly->x2;
			u8 *c0 = &poly->r0;
			u8 *c1 = c0;

			GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
			LineSwapSourceVerts(&p0, &p1, &c0, &c1);
			MakeLineArray(firstVertex, p0, p1);
			MakeTexcoordLineZero(firstVertex, 0);
			MakeColourLine(firstVertex, shadeTexOn, c0, c1);

			TriangulateQuad();

			g_vertexIndex += 6;
#if defined(DEBUG_POLY_COUNT)
			polygon_count++;
#endif
		}

		return 5;
	}
	case 0xc:
	{
		int i;
		LINE_F4 *poly = (LINE_F4 *)polyTag;

		AddSplit(semiTrans, false);

		{
			VERTTYPE *p0 = &poly->x0;
			VERTTYPE *p1 = &poly->x1;
			u8 *c0 = &poly->r0;
			u8 *c1 = c0;

			GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
			LineSwapSourceVerts(&p0, &p1, &c0, &c1);
			MakeLineArray(firstVertex, p0, p1);
			MakeTexcoordLineZero(firstVertex, 0);
			MakeColourLine(firstVertex, shadeTexOn, c0, c1);

			TriangulateQuad();

			g_vertexIndex += 6;
#if defined(DEBUG_POLY_COUNT)
			polygon_count++;
#endif
		}

		{
			VERTTYPE *p0 = &poly->x1;
			VERTTYPE *p1 = &poly->x2;
			u8 *c0 = &poly->r0;
			u8 *c1 = c0;

			GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
			LineSwapSourceVerts(&p0, &p1, &c0, &c1);
			MakeLineArray(firstVertex, p0, p1);
			MakeTexcoordLineZero(firstVertex, 0);
			MakeColourLine(firstVertex, shadeTexOn, c0, c1);

			TriangulateQuad();

			g_vertexIndex += 6;
#if defined(DEBUG_POLY_COUNT)
			polygon_count++;
#endif
		}

		{
			VERTTYPE *p0 = &poly->x2;
			VERTTYPE *p1 = &poly->x3;
			u8 *c0 = &poly->r0;
			u8 *c1 = c0;

			GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
			LineSwapSourceVerts(&p0, &p1, &c0, &c1);
			MakeLineArray(firstVertex, p0, p1);
			MakeTexcoordLineZero(firstVertex, 0);
			MakeColourLine(firstVertex, shadeTexOn, c0, c1);

			TriangulateQuad();

			g_vertexIndex += 6;
#if defined(DEBUG_POLY_COUNT)
			polygon_count++;
#endif
		}

		return 6;
	}
	}
	return 0;
}

static int ProcessGouraudLines(P_TAG *polyTag)
{
	const bool shadeTexOn = true;
	const bool semiTrans = (polyTag->code & 2);
	const int primSubType = polyTag->code & 0x0C;

	switch (primSubType)
	{
	case 0x0:
	{
		LINE_G2 *poly = (LINE_G2 *)polyTag;

		AddSplit(semiTrans, false);

		VERTTYPE *p0 = &poly->x0;
		VERTTYPE *p1 = &poly->x1;
		u8 *c0 = &poly->r0;
		u8 *c1 = &poly->r1;

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		LineSwapSourceVerts(&p0, &p1, &c0, &c1);
		MakeLineArray(firstVertex, p0, p1);
		MakeTexcoordLineZero(firstVertex, 0);
		MakeColourLine(firstVertex, shadeTexOn, c0, c1);

		TriangulateQuad();

		g_vertexIndex += 6;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 4;
	}
	case 0x8:
	{
		// TODO: LINE_G3
		return 7;
	}
	case 0xC:
	{
		// TODO: LINE_G4
		return 9;
	}
	}
	return 0;
}

static int ProcessFlatPoly(P_TAG *polyTag)
{
	const bool shadeTexOn = (polyTag->code & 1) == 0;
	const bool semiTrans = (polyTag->code & 2);
	const int primSubType = polyTag->code & 0x0C;

	switch (primSubType)
	{
	case 0x0:
	{
		POLY_F3 *poly = (POLY_F3 *)polyTag;

		AddSplit(semiTrans, false);

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		MakeVertexTriangle(firstVertex, &poly->x0, &poly->x1, &poly->x2);
		MakeTexcoordTriangleZero(firstVertex, 0);
		MakeColourTriangle(firstVertex, shadeTexOn, &poly->r0, &poly->r0, &poly->r0);

		g_vertexIndex += 3;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 4;
	}
	case 0x4:
	{
		POLY_FT3 *poly = (POLY_FT3 *)polyTag;
		activeDrawEnv.tpage = poly->tpage;

		// It is an official hack from SCE devs to not use DR_TPAGE and instead use null polygon
		if (!IsNull(poly))
		{
			AddSplit(semiTrans, true);

			GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
			MakeVertexTriangle(firstVertex, &poly->x0, &poly->x1, &poly->x2);
			MakeTexcoordTriangle(firstVertex, &poly->u0, &poly->u1, &poly->u2, poly->tpage, poly->clut,
			                     GET_TPAGE_DITHER(activeDrawEnv.tpage) || activeDrawEnv.dtd);
			MakeColourTriangle(firstVertex, shadeTexOn, &poly->r0, &poly->r0, &poly->r0);

			g_vertexIndex += 3;

#if defined(DEBUG_POLY_COUNT)
			polygon_count++;
#endif
		}
		return 7;
	}
	case 0x8:
	{
		POLY_F4 *poly = (POLY_F4 *)polyTag;

		AddSplit(semiTrans, false);

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		MakeVertexQuad(firstVertex, &poly->x0, &poly->x1, &poly->x3, &poly->x2);
		MakeTexcoordQuadZero(firstVertex, 0);
		MakeColourQuad(firstVertex, shadeTexOn, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		g_vertexIndex += 6;
#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 5;
	}
	case 0xC:
	{
		POLY_FT4 *poly = (POLY_FT4 *)polyTag;
		activeDrawEnv.tpage = poly->tpage;

		AddSplit(semiTrans, true);

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		MakeVertexQuad(firstVertex, &poly->x0, &poly->x1, &poly->x3, &poly->x2);
		MakeTexcoordQuad(firstVertex, &poly->u0, &poly->u1, &poly->u3, &poly->u2, poly->tpage, poly->clut,
		                 GET_TPAGE_DITHER(activeDrawEnv.tpage) || activeDrawEnv.dtd);
		MakeColourQuad(firstVertex, shadeTexOn, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		g_vertexIndex += 6;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 9;
	}
	}
	return 0;
}

static int ProcessGouraudPoly(P_TAG *polyTag)
{
	const bool shadeTexOn = true;
	const bool semiTrans = (polyTag->code & 2);
	const int primSubType = polyTag->code & 0x0C;

	switch (primSubType)
	{
	case 0x0:
	{
		POLY_G3 *poly = (POLY_G3 *)polyTag;

		AddSplit(semiTrans, false);

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		MakeVertexTriangle(firstVertex, &poly->x0, &poly->x1, &poly->x2);
		MakeTexcoordTriangleZero(firstVertex, 1);
		MakeColourTriangle(firstVertex, shadeTexOn, &poly->r0, &poly->r1, &poly->r2);

		g_vertexIndex += 3;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 6;
	}
	case 0x4:
	{
		POLY_GT3 *poly = (POLY_GT3 *)polyTag;
		activeDrawEnv.tpage = poly->tpage;

		AddSplit(semiTrans, true);

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		MakeVertexTriangle(firstVertex, &poly->x0, &poly->x1, &poly->x2);
		MakeTexcoordTriangle(firstVertex, &poly->u0, &poly->u1, &poly->u2, poly->tpage, poly->clut, GET_TPAGE_DITHER(activeDrawEnv.tpage) || activeDrawEnv.dtd);
		MakeColourTriangle(firstVertex, shadeTexOn, &poly->r0, &poly->r1, &poly->r2);

		g_vertexIndex += 3;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 9;
	}
	case 0x8:
	{
		POLY_G4 *poly = (POLY_G4 *)polyTag;

		AddSplit(semiTrans, false);

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		MakeVertexQuad(firstVertex, &poly->x0, &poly->x1, &poly->x3, &poly->x2);
		MakeTexcoordQuadZero(firstVertex, 1);
		MakeColourQuad(firstVertex, shadeTexOn, &poly->r0, &poly->r1, &poly->r3, &poly->r2);

		TriangulateQuad();

		g_vertexIndex += 6;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 8;
	}
	case 0xC:
	{
		POLY_GT4 *poly = (POLY_GT4 *)polyTag;
		activeDrawEnv.tpage = poly->tpage;

		AddSplit(semiTrans, true);

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		MakeVertexQuad(firstVertex, &poly->x0, &poly->x1, &poly->x3, &poly->x2);
		MakeTexcoordQuad(firstVertex, &poly->u0, &poly->u1, &poly->u3, &poly->u2, poly->tpage, poly->clut,
		                 GET_TPAGE_DITHER(activeDrawEnv.tpage) || activeDrawEnv.dtd);
		MakeColourQuad(firstVertex, shadeTexOn, &poly->r0, &poly->r1, &poly->r3, &poly->r2);

		TriangulateQuad();

		g_vertexIndex += 6;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 12;
	}
	}
	return 0;
}

static int ProcessTileAndSprt(P_TAG *polyTag)
{
	// NOTE: TILE does not support switching shadeTex on real PSX
	const bool shadeTexOn = (polyTag->code & 1) == 0;
	const bool semiTrans = (polyTag->code & 2);

	switch (polyTag->code & 0xFD)
	{
	case 0x60:
	{
		TILE *poly = (TILE *)polyTag;

		AddSplit(semiTrans, false);

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		MakeVertexRect(firstVertex, &poly->x0, poly->w, poly->h);
		MakeTexcoordQuadZero(firstVertex, 0);
		MakeColourQuad(firstVertex, shadeTexOn, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		g_vertexIndex += 6;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 3;
	}
	case 0x64:
	{
		SPRT *poly = (SPRT *)polyTag;

		AddSplit(semiTrans, true);

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		MakeVertexRect(firstVertex, &poly->x0, poly->w, poly->h);
		MakeTexcoordRect(firstVertex, &poly->u0, activeDrawEnv.tpage, poly->clut, poly->w, poly->h);
		MakeColourQuad(firstVertex, shadeTexOn, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		g_vertexIndex += 6;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 4;
	}
	case 0x68:
	{
		TILE_1 *poly = (TILE_1 *)polyTag;

		AddSplit(semiTrans, false);

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		MakeVertexRect(firstVertex, &poly->x0, 1, 1);
		MakeTexcoordQuadZero(firstVertex, 0);
		MakeColourQuad(firstVertex, true, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		g_vertexIndex += 6;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 2;
	}
	case 0x70:
	{
		TILE_8 *poly = (TILE_8 *)polyTag;

		AddSplit(semiTrans, false);

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		MakeVertexRect(firstVertex, &poly->x0, 8, 8);
		MakeTexcoordQuadZero(firstVertex, 0);
		MakeColourQuad(firstVertex, true, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		g_vertexIndex += 6;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 2;
	}
	case 0x74:
	{
		SPRT_8 *poly = (SPRT_8 *)polyTag;

		AddSplit(semiTrans, true);

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		MakeVertexRect(firstVertex, &poly->x0, 8, 8);
		MakeTexcoordRect(firstVertex, &poly->u0, activeDrawEnv.tpage, poly->clut, 8, 8);
		MakeColourQuad(firstVertex, shadeTexOn, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		g_vertexIndex += 6;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 3;
	}
	case 0x78:
	{
		TILE_16 *poly = (TILE_16 *)polyTag;

		AddSplit(semiTrans, false);

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		MakeVertexRect(firstVertex, &poly->x0, 16, 16);
		MakeTexcoordQuadZero(firstVertex, 0);
		MakeColourQuad(firstVertex, true, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		g_vertexIndex += 6;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 2;
	}
	case 0x7C:
	{
		SPRT_16 *poly = (SPRT_16 *)polyTag;

		AddSplit(semiTrans, true);

		GrVertex *firstVertex = &g_vertexBuffer[g_vertexIndex];
		MakeVertexRect(firstVertex, &poly->x0, 16, 16);
		MakeTexcoordRect(firstVertex, &poly->u0, activeDrawEnv.tpage, poly->clut, 16, 16);
		MakeColourQuad(firstVertex, shadeTexOn, &poly->r0, &poly->r0, &poly->r0, &poly->r0);

		TriangulateQuad();

		g_vertexIndex += 6;

#if defined(DEBUG_POLY_COUNT)
		polygon_count++;
#endif
		return 3;
	}
	}
	return 0;
}

static int ProcessDrawEnv(P_TAG *polyTag)
{
	const u32 *codePtr = (u32 *)&polyTag->pad0;
	int processedLongs = 0;
	for (int i = 0; i < polyTag->len; ++i)
	{
		const u32 code = codePtr[i];
		const int primType = code >> 24 & 0xF0;
		const int primSubType = code >> 24 & 0x0F;

		// NOTE(aalhendi): CTR can pack draw-env commands, tagless geometry,
		// and more draw-env commands into one OT entry. Stop at the first
		// non-E command so ParseTaglessPrimitive owns the geometry payload.
		if (primType != 0xE0)
			return processedLongs;

		switch (primSubType)
		{
		case 0x1:
		{
			// DR_TPAGE
			activeDrawEnv.tpage = (code & 0x1FF);
			activeDrawEnv.dtd = (code >> 9) & 1;
			// NOTE(aalhendi): ctr-native local divergence. CTR uses
			// DR_TPAGE packets for blend changes inside the OT; native
			// target selection must stay owned by DRAWENV.
			// activeDrawEnv.dfe = (code >> 10) & 1;
			break;
		}
		case 0x2:
		{
			// DR_TWIN
			activeDrawEnv.tw.w = (code & 0x1F);
			activeDrawEnv.tw.h = ((code >> 5) & 0x1F);
			activeDrawEnv.tw.x = ((code >> 10) & 0x1F);
			activeDrawEnv.tw.y = ((code >> 15) & 0x1F);
			break;
		}
		case 0x3:
		{
			// DR_AREA
			activeDrawEnv.clip.x = code & 1023;
			activeDrawEnv.clip.y = (code >> 10) & 1023;
			break;
		}
		case 0x4:
		{
			// DR_AREA (second part)
			activeDrawEnv.clip.w = code & 1023;
			activeDrawEnv.clip.h = (code >> 10) & 1023;

			activeDrawEnv.clip.w -= activeDrawEnv.clip.x;
			activeDrawEnv.clip.h -= activeDrawEnv.clip.y;
			break;
		}
		case 0x5:
		{
			// DR_OFFSET
			// TODO
			activeDrawEnv.ofs[0] = code & 2047;
			activeDrawEnv.ofs[1] = (code >> 11) & 2047;
			break;
		}
		case 0x6:
		{
			SetPSXMaskState(code);
			break;
		}
		case 0:
			// NOTE(aalhendi): ctr-native local divergence for CTR OTs. A zero
			// word can be terminal draw-env padding, or the tag word for the
			// next primitive packed into the same OT entry.
			// return processedLongs;
			if (i + 1 != polyTag->len)
				return processedLongs;
			break;
		}
		++processedLongs;
	}

	return processedLongs;
}

static void ProcessDrawEnvCommand(u32 code)
{
	const int primSubType = code >> 24 & 0x0F;

	switch (primSubType)
	{
	case 0x1:
		activeDrawEnv.tpage = (code & 0x1FF);
		activeDrawEnv.dtd = (code >> 9) & 1;
		break;
	case 0x2:
		activeDrawEnv.tw.w = (code & 0x1F);
		activeDrawEnv.tw.h = ((code >> 5) & 0x1F);
		activeDrawEnv.tw.x = ((code >> 10) & 0x1F);
		activeDrawEnv.tw.y = ((code >> 15) & 0x1F);
		break;
	case 0x3:
		activeDrawEnv.clip.x = code & 1023;
		activeDrawEnv.clip.y = (code >> 10) & 1023;
		break;
	case 0x4:
		activeDrawEnv.clip.w = code & 1023;
		activeDrawEnv.clip.h = (code >> 10) & 1023;
		activeDrawEnv.clip.w -= activeDrawEnv.clip.x;
		activeDrawEnv.clip.h -= activeDrawEnv.clip.y;
		break;
	case 0x5:
		activeDrawEnv.ofs[0] = code & 2047;
		activeDrawEnv.ofs[1] = (code >> 11) & 2047;
		break;
	case 0x6:
		SetPSXMaskState(code);
		break;
	}
}

static int ProcessPsyXPrims(P_TAG *polyTag)
{
	const int primType = polyTag->code & 0xF0;
	const int primSubType = polyTag->code & 0x0F;

	switch (primSubType)
	{
	case 0x01:
	{
		DR_PSYX_TEX *psytex = (DR_PSYX_TEX *)polyTag;
		overrideTexture = psytex->code[0] & 0xFFFFFF;
		overrideTextureWidth = psytex->code[1] & 0xFFF;
		overrideTextureHeight = psytex->code[1] >> 16 & 0xFFF;
		return 2;
	}
	case 0x02:
	{
		// [A] Psy-X custom debug marker packet
		DR_PSYX_DBGMARKER *psydbg = (DR_PSYX_DBGMARKER *)polyTag;
		currentSplitDebugText = psydbg->text;
		return 2;
	}
	}

	return 0;
}

// Processes primitive
// returns processed primitive primLength in longs
int ParsePrimitive(P_TAG *polyTag)
{
	const int primType = polyTag->code & 0xF0;

	int primLength = 0;
	bool handledZeroLength = false;

	switch (primType)
	{
	case 0x00:
	{
		const int primSubType = polyTag->code & 0x0F;
		const u32 *codePtr = (u32 *)&polyTag->pad0;
		// NOTE(aalhendi): ctr-native local divergence. CTR RenderWeather can
		// emit a retail length-2 zero packet when weather is enabled but the
		// level has no fill-mode payload. The PSX consumes it by tag length;
		// the native parser must advance past it too.
		if (polyTag->len == 2 && codePtr[0] == 0 && codePtr[1] == 0)
		{
			primLength = 2;
		}
		else if (polyTag->len == 0 && *(u32 *)polyTag == 0)
		{
			// CTR ghost transparency packets include raw GPU NOP words between
			// draw-mode changes and triangle commands. They consume exactly one
			// command word; ParsePrimitivesLinkedList adds P_LEN to the return.
			handledZeroLength = true;
		}
		else if (primSubType == 0x0)
		{
			primLength = 3;
		}
		else if (primSubType == 0x1)
		{
			DR_MOVE *drmove = (DR_MOVE *)polyTag;

			const int y = drmove->code[3] >> 0x10 & 0xFFFF;
			const int x = drmove->code[3] & 0xFFFF;

			RECT16 rect;
			*(u32 *)&rect.x = *(u32 *)&drmove->code[2];
			*(u32 *)&rect.w = *(u32 *)&drmove->code[4];

			MoveImage(&rect, x, y);
			primLength = 5;
		}
		else if (primSubType == 0x2)
		{
			// NOTE(aalhendi): ctr-native local divergence. CTR emits retail
			// FILL packets in OTs; the old PsyCross parser did not consume them, which caused
			// zero-length primitive spam.
			TILE *fill = (TILE *)polyTag;
			RECT16 rect;

			rect.x = fill->x0;
			rect.y = fill->y0;
			rect.w = fill->w;
			rect.h = fill->h;

			ClearImage(&rect, fill->r0, fill->g0, fill->b0);
			primLength = 3;
		}
		break;
	}
	case 0x20:
		// Flat polygons
		primLength = ProcessFlatPoly(polyTag);
		break;
	case 0x30:
		// Gouraud shaded polygons
		primLength = ProcessGouraudPoly(polyTag);
		break;
	case 0x40:
		// Flat (single colour) Lines
		primLength = ProcessFlatLines(polyTag);
		break;
	case 0x50:
		// Gouraud lines
		primLength = ProcessGouraudLines(polyTag);
		break;
	case 0x60:
	case 0x70:
		// TILE and SPRT
		primLength = ProcessTileAndSprt(polyTag);
		break;
	case 0xA0:
		// DR_LOAD
		{
			DR_LOAD *drload = (DR_LOAD *)polyTag;

			RECT16 rect;
			*(u32 *)&rect.x = *(u32 *)&drload->code[1];
			*(u32 *)&rect.w = *(u32 *)&drload->code[2];

			LoadImage(&rect, (u_long *)drload->p);

			// TODO(aalhendi): Audit whether CTR ever appends additional GPU
			// commands after a DR_LOAD payload in the same packet.
		}
		primLength = getlen(polyTag);
		break;
	case 0xB0:
		// [A] Psy-X custom primitives
		primLength = ProcessPsyXPrims(polyTag);
		break;
	case 0xE0:
		// Draw Env setup
		primLength = ProcessDrawEnv(polyTag);
		break;
		// default:
		//	NATIVE_GPU_ERROR("got %0x primitive\n", primType);
	}

	if (primLength == 0 && !handledZeroLength)
	{
		NATIVE_GPU_ERROR("Unhandled zero length %0x primitive\n", primType);
	}

	return primLength;
}

int ParseTaglessPrimitive(u32 *command)
{
	const u32 code = *command;
	const int primType = (code >> 24) & 0xF0;

	if (code == 0)
		return 1;

	if (primType == 0xE0)
	{
		ProcessDrawEnvCommand(code);
		return 1;
	}

	P_TAG *polyTag = (P_TAG *)(command - P_LEN);
	int primLength = ParsePrimitive(polyTag);

	if (primLength == 0)
	{
		NATIVE_GPU_ERROR("Unhandled tagless primitive %08x\n", code);
		return 1;
	}

	return primLength;
}
