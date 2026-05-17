#include <common.h>

extern struct Ovr233_Credits_BSS *creditsBSS;

void CS_Credits_AnimateCreditGhost(struct Instance *dst, struct Instance *src, int index)
{
	struct CreditsObj *co = &creditsBSS->creditsObj;

	dst->animFrame = src->animFrame;
	dst->animIndex = src->animIndex;

	dst->matrix = src->matrix;

	short scale = 0x1000 + (index + 1) * 300;

	dst->scale[0] = scale;
	dst->scale[1] = scale;
	dst->scale[2] = scale;

	dst->flags &= ~0x80;
	if ((int)dst->model == 0)
		dst->flags |= 0x80;

	dst->alphaScale = (index + 1) * 630;

	struct Model *localModel = (struct Model *)co->data_0x18_0x5[index].data;
	dst->model = localModel;

	struct Model *srcModel = src->model;
	int *dstModelInts = (int *)localModel;
	int *srcModelInts = (int *)srcModel;
	dstModelInts[0] = srcModelInts[0];
	dstModelInts[1] = srcModelInts[1];
	dstModelInts[2] = srcModelInts[2];
	dstModelInts[3] = srcModelInts[3];
	dstModelInts[4] = srcModelInts[4];
	dstModelInts[5] = srcModelInts[5];

	localModel->headers = (struct ModelHeader *)co->data_0x80_0x5[index].data;

	short srcNumHeaders = srcModel->numHeaders;
	if (srcNumHeaders > 0)
	{
		struct ModelHeader *dstHeaders = localModel->headers;
		struct ModelHeader *srcHeaders = srcModel->headers;

		for (int i = 0; i < srcNumHeaders; i++)
		{
			int *d = (int *)&dstHeaders[i];
			int *s = (int *)&srcHeaders[i];
			d[0] = s[0];
			d[1] = s[1];
			d[2] = s[2];
			d[3] = s[3];
			d[4] = s[4];
			d[5] = s[5];
			d[6] = s[6];
			d[7] = s[7];
			d[8] = s[8];
			d[9] = s[9];
			d[10] = s[10];
			d[11] = s[11];
			d[12] = s[12];
			d[13] = s[13];
			d[14] = s[14];
			d[15] = s[15];
		}
	}
}
