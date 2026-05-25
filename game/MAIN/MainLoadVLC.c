#include <common.h>

void MainLoadVLC_Callback();

void MainLoadVLC(void)
{
	// VLC is not loaded
	sdata->bool_IsLoaded_VlcTable = 0;

	// This table is passed as parameter to DecDCTvlc2
	DECOMP_LOAD_AppendQueue(0, LT_SETADDR, BI_VLCTABLE, 0, MainLoadVLC_Callback);
}
