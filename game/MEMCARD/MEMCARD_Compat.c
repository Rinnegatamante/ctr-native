#include <common.h>

static char *MEMCARD_NativePath(char *save_name)
{
	if (strncmp(save_name, "bu00:", 5) == 0)
		return save_name + 5;

	if (strncmp(save_name, "bu01:", 5) == 0)
		return save_name + 5;

	return save_name;
}

// NOTE(aalhendi): ctr-native stubs host-unsupported card directory ops here;
// the retail implementations live in MEMCARD_16/18/21-25 and are not included.
void MEMCARD_GetFreeBytes(int slotIdx)
{
	(void)slotIdx;
	sdata->memoryCard_SizeRemaining = 0x1e000;
}

u8 MEMCARD_GetInfo(int slotIdx)
{
	(void)slotIdx;

	// NOTE(aalhendi): Native treats the host save directory as an inserted card;
	// returning NEWCARD makes the retail RefreshCard path refresh file presence.
	return MC_RETURN_NEWCARD;
}

u8 MEMCARD_Format(int slotIdx)
{
	(void)slotIdx;
	return MC_RETURN_IOE;
}

int MEMCARD_IsFile(int slotIdx, char *save_name)
{
	FILE *file;
	char *path;

	(void)slotIdx;

	path = MEMCARD_NativePath(save_name);
	file = fopen(path, "rb");
	if (file == NULL)
		return MC_RETURN_NODATA;

	fclose(file);
	return MC_RETURN_IOE;
}

char *MEMCARD_FindFirstGhost(int slotIdx, char *srcString)
{
	(void)slotIdx;
	(void)srcString;
	return NULL;
}

char *MEMCARD_FindNextGhost(void)
{
	return NULL;
}

u8 MEMCARD_EraseFile(int slotIdx, char *srcString)
{
	char *path;

	(void)slotIdx;
	path = MEMCARD_NativePath(srcString);
	return remove(path) == 0 ? MC_RETURN_IOE : MC_RETURN_NODATA;
}
