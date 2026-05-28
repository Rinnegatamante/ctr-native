#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 PS1 path 0x8003e29c-0x8003e344;
// CTR_NATIVE reads host files directly.
u8 MEMCARD_Load(int slotIdx, char *name, u8 *ptrMemcard, int memcardFileSize, u32 param5)
{
#ifdef CTR_NATIVE
	char path[0x40];
	char *nameNoDevice;
	FILE *file;
	size_t readBytes;
	int checksumResult;

	(void)slotIdx;
	(void)param5;

	nameNoDevice = name;
	if (strncmp(nameNoDevice, "bu00:", 5) == 0)
		nameNoDevice += 5;
	else if (strncmp(nameNoDevice, "bu01:", 5) == 0)
		nameNoDevice += 5;

	snprintf(path, sizeof(path), "%s", nameNoDevice);

	file = fopen(path, "rb");
	if (file == NULL)
		return MC_RETURN_NODATA;

	if (fseek(file, 0x100, SEEK_SET) != 0)
	{
		fclose(file);
		return MC_RETURN_TIMEOUT;
	}

	readBytes = fread(ptrMemcard, 1, memcardFileSize, file);
	fclose(file);

	if (readBytes != (size_t)memcardFileSize)
		return MC_RETURN_TIMEOUT;

	sdata->crc16_checkpoint_byteIndex = 0;
	sdata->crc16_checkpoint_status = 0;
	do
	{
		checksumResult = MEMCARD_ChecksumLoad(ptrMemcard, memcardFileSize);
	} while (checksumResult == MC_RETURN_PENDING);

	return checksumResult == MC_RETURN_IOE ? MC_RETURN_IOE : MC_RETURN_TIMEOUT;
#else
	if (sdata->memcard_stage != MC_STAGE_IDLE)
		return MC_RETURN_TIMEOUT;

	if (MEMCARD_NewTask(slotIdx, name, ptrMemcard, memcardFileSize, 0) != 0)
		return MC_RETURN_TIMEOUT;

	sdata->memcard_fd = open(sdata->s_memcardFileCurr, FASYNC | FREAD);

	if (sdata->memcard_fd == -1)
	{
		MEMCARD_CloseFile();
		return MC_RETURN_NODATA;
	}

	if ((param5 & 2) != 0)
		sdata->memcardStatusFlags |= 8;
	else
		sdata->memcardStatusFlags &= ~8;

	sdata->memcard_stage = MC_STAGE_LOAD_PART0_START;
	return MEMCARD_ReadFile(0, 0x80);
#endif
}
