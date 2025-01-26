/*
*
*	SEGA Saturn SGM save Unpacker - (c) 2025 by Bucanero - www.bucanero.com.ar
*
* This tool is based on the Save Game Manager (SGM) compressed format by Rockin'-B (www.rockin-b.de)
*
*/

#include "iofile.c"
#include "bup_header.h"
#include "bzlib.h"
#include "sgm-types.h"


int export_BUP(const SGMFile* sgmfile)
{
	FILE *fp;
	char fname[32];
	vmem_bup_header_t header = {0};
	jo_backup_date date = {
		.year = ES16(sgmfile->header.year) - 1980,
		.month = sgmfile->header.month,
		.day = sgmfile->header.day,
		.time = sgmfile->header.hour,
		.min = sgmfile->header.minute,
		.week = sgmfile->header.weekday,
	};

	memcpy(header.magic, VMEM_MAGIC_STRING, VMEM_MAGIC_STRING_LEN);
	memcpy(header.dir.filename, sgmfile->header.name, 11);
	memcpy(header.dir.comment, sgmfile->header.comment, 10);
	header.dir.datasize = sgmfile->header.size;
	header.dir.language = sgmfile->header.language;
	header.dir.date = bup_setdate(&date);	
	header.dir.date = header.date = ES32(header.dir.date);
	header.dir.blocksize = calculateUsedBlocks(sgmfile->size, SAT_CLUSTER_SIZE);

	snprintf(fname, sizeof(fname), "%s%s", sgmfile->header.name, BUP_EXTENSION);
	fp = fopen(fname, "wb");
	if (!fp)
	{
		printf("[X] ERROR: Couldn't create file! (%s)\n", fname);
		return 0;
	}

	fwrite(&header, sizeof(vmem_bup_header_t), 1, fp);
	fwrite(sgmfile->data, sgmfile->size, 1, fp);
	fclose(fp);

	printf("    > %s \t %d-%02d-%02d %02d:%02d \t %d bytes \t OK!\n", fname,
		date.year + 1980, date.month, date.day, date.time, date.min, sgmfile->size);

	return (1);
}

int unpack_SGM(const u8 *data, u32 len, u32 files)
{
	u32 exp = 0;
	SGMFile sgmfile;

	for (u32 offset = 4; files && offset < len; files--)
	{
		memcpy(&sgmfile.header, data + offset, sizeof(MemoryFile));
		sgmfile.size = ES32(sgmfile.header.size);
		sgmfile.data = (u8*)(data + offset + sizeof(MemoryFile));
		exp += export_BUP(&sgmfile);
		offset += sizeof(MemoryFile) + sgmfile.size;
	}

	return exp;
}

void print_usage(const char* argv0)
{
	printf("USAGE: %s filename\n\n", argv0);
	return;
}

int main(int argc, char **argv)
{
	size_t len;
	u8 *data;
	u8 *unpacked;
	u32 ulen = 0x100000;
	u32 files;

	printf("\nSEGA Saturn SGM save unpacker 0.1.0 - (c) 2025 by Bucanero\n\n");

	if (--argc < 1)
	{
		print_usage(argv[0]);
		return -1;
	}

	if (read_buffer(argv[1], &data, &len) != 0)
	{
		printf("[*] Could Not Access The File (%s)\n", argv[1]);
		return -1;
	}

	// detect BZip2 save
	if (memcmp(data, "BZ", 2) != 0)
	{
		printf("[X] This is not a SGM packed save file! (%s)\n", argv[1]);
		return -1;
	}

	unpacked = (u8*)malloc(ulen);

	// unpack BZip2 save
	if (!unpacked || BZ2_bzBuffToBuffDecompress((char*) unpacked, &ulen, (char*) data, len, 0, 0) != BZ_OK)
	{
		printf("[X] Error unpacking the save file!\n");
		return -1;
	}
	free(data);

	files = *(u32*)(unpacked);
	printf("[*] Unpacking %s...\n[i] Packed Files: %d\n", argv[1], ES32(files));
	files = unpack_SGM(unpacked, ulen, ES32(files));

	printf("[i] Successfully Unpacked %d files.\n\n", files);
	free(unpacked);

	return 0;
}
