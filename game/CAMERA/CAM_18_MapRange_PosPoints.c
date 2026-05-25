#include <common.h>

int CAM_MapRange_PosPoints(s16 *pos1, s16 *pos2, s16 *currPos)
{
	int dx = pos1[0] - pos2[0];
	int dy = pos1[1] - pos2[1];
	int dz = pos1[2] - pos2[2];
	int length = SquareRoot0(dx * dx + dy * dy + dz * dz);

	if (length == 0)
		return 0;

	int nx = (dx << 12) / length;
	int ny = (dy << 12) / length;
	int nz = (dz << 12) / length;

	int cx = currPos[0] - pos1[0];
	int cy = currPos[1] - pos1[1];
	int cz = currPos[2] - pos1[2];

	return (nx * cx + ny * cy + nz * cz) >> 12;
}
