#include <common.h>

static struct LevVertex *sCollFixedLoadScratchpadVertsVertexArray;
static struct QuadBlock *sCollFixedLoadScratchpadVertsQuad;

static void COLL_FIXED_QUADBLK_SetLoadScratchpadVertsContext(struct ScratchpadStruct *sps, struct QuadBlock *quad)
{
	// NOTE(aalhendi): Retail passes these through implicit MIPS registers t8/t9.
	// Native records that register state explicitly before calling the loader.
	sCollFixedLoadScratchpadVertsVertexArray = sps->ptr_mesh_info->ptrVertexArray;
	sCollFixedLoadScratchpadVertsQuad = quad;
}

void COLL_FIXED_QUADBLK_LoadScratchpadVerts(struct ScratchpadStruct *sps)
{
	struct LevVertex *ptrVert = sCollFixedLoadScratchpadVertsVertexArray;
	struct QuadBlock *ptrQuad = sCollFixedLoadScratchpadVertsQuad;
	struct BspSearchVertex *bsv;
	struct LevVertex *vertCurr;
	u16 *index;

	bsv = &sps->bspSearchVert[0];
	*(u32 *)&sps->unkVecE8[2] = *(u32 *)&ptrQuad->index[2];

	for (index = (u16 *)&ptrQuad->index[0]; index < (u16 *)&ptrQuad->index[9]; index++, bsv++)
	{
		vertCurr = &ptrVert[*index];
		bsv->pLevelVertex = vertCurr;
		*(int *)&bsv->pos[0] = *(int *)&vertCurr->pos[0];
		*(int *)&bsv->pos[2] = *(int *)&vertCurr->pos[2];
	}
}
