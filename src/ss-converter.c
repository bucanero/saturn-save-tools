/*
*
*	SEGA Saturn Save Converter - (c) 2025 by Bucanero - www.bucanero.com.ar
*
*/

#include "iofile.c"
#include "base64.c"
#include "bup_header.h"
#include "sgm-types.h"
#include "xml.h"

#define XML_HEADER  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"

typedef struct {
    char* filename;
    jo_backup_file header;
    uint32_t size;
    uint8_t *data;
} SaturnSave;

typedef struct {
    char *type;
    int (*formatChecker)(const uint8_t *, size_t);
    SaturnSave* (*saveLoader)(const uint8_t *, size_t);
} save_format_t;


/* This is the basic CRC-32 calculation with some optimization but no table lookup. */
u32 crc32b(const u8 *data, int size)
{
    int j;
    u32 crc = 0xFFFFFFFF;

    while (size--){
        crc ^= *data++;          // XOR with next byte.
        for (j=0; j<8; j++){     // Do eight times.
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }
    return ~crc;
}

void print_details(const SaturnSave* save)
{
    jo_backup_date date;
    const char* lang[] = {"JP", "EN", "FR", "DE", "ES", "IT"};

    bup_getdate(ES32(save->header.date), &date);
    printf("    > Filename:\t %s\n", save->header.filename);
    printf("    > Comment :\t %s\n", save->header.comment);
    printf("    > Language:\t %s\n", (save->header.language > backup_italiano) ? "??" : lang[save->header.language]);
    printf("    > Date:    \t %d-%02d-%02d %02d:%02d\n", date.year+1980, date.month, date.day, date.time, date.min);
    printf("    > Size:    \t %d bytes\n\n", save->size);
    return;
}

int check_B64_save(const u8 *src_data, size_t len)
{
    return (len > 56 && src_data[54] == '=' && src_data[55] == '=');
}

SaturnSave *load_B64_save(const u8 *src_data, size_t len)
{
    size_t dlen;
    uint8_t *data;
    SaturnSave *save;
    MemoryFile header;
	jo_backup_date date;

    data = base64_decode(src_data, len, &dlen);
    if (!data || dlen != sizeof(MemoryFile))
    {
        printf("[X] Error loading Base64 save file!\n");
        free(data);
        return NULL;
    }

    memcpy(&header, data, sizeof(MemoryFile));
    free(data);

    save = (SaturnSave*)malloc(sizeof(SaturnSave));
    memset(save, 0, sizeof(SaturnSave));
    memcpy(save->header.filename, header.name, 11);
    memcpy(save->header.comment, header.comment, 10);
    save->header.datasize = header.size;
    save->header.language = header.language;

    date.year = ES16(header.year) - 1980;
    date.month = header.month;
    date.day = header.day;
    date.time = header.hour;
    date.min = header.minute;
    date.week = header.weekday;
    save->header.date = bup_setdate(&date);	
    save->header.date = ES32(save->header.date);

    data = (uint8_t*) strchr((char*) src_data, '\n');
    if (!data)
    {
        printf("[X] Error loading Base64 save file!\n");
        free(save);
        return NULL;
    }
    save->data = base64_decode(data, len - (data - src_data), &dlen);
    save->size = dlen;

    return save;
}

int check_XML_save(const u8 *src_data, size_t len)
{
    return (len > strlen(XML_HEADER) && memcmp(src_data, XML_HEADER, strlen(XML_HEADER)) == 0);
}

SaturnSave *load_XML_save(const u8 *src_data, size_t len)
{
    size_t dlen, items;
    SaturnSave *save;
	jo_backup_date date;
	struct xml_document* document;
    struct xml_node* xml_save;
    uint8_t *xml = malloc(len);

    // check the result of `xml_parse_document', if it's 0
    // then the source could not be parsed.
    dlen = strlen(XML_HEADER);
    memcpy(xml, src_data + dlen, len - dlen);
    document = xml_parse_document(xml, len - dlen);
    if (!document)
    {
        printf("[X] Error loading XML save file!\n");
        free(xml);
        return NULL;
    }

    save = (SaturnSave*)malloc(sizeof(SaturnSave));
    memset(save, 0, sizeof(SaturnSave));

    xml_save = xml_node_child(xml_document_root(document), 0);
    items = xml_node_children(xml_save);

    for(size_t i = 0; i < items; i++)
    {
        struct xml_node* item = xml_node_child(xml_save, i);
        char* name = (char*) xml_easy_name(item);
        char* value = (char*) xml_easy_content(item);

        if (strcmp(name, "name_binary") == 0)
        {
            free(name);
            name = (char*) base64_decode((uint8_t*) value, strlen(value), &dlen);
            strncpy((char*) save->header.filename, name ? name : "", 11);
        }
        else if (strcmp(name, "comment_binary") == 0)
        {
            free(name);
            name = (char*) base64_decode((uint8_t*) value, strlen(value), &dlen);
            strncpy((char*) save->header.comment, name ? name : "", 10);
        }
        else if (strcmp(name, "language_code") == 0)
        {
            save->header.language = atoi(value);
        }
        else if (strcmp(name, "size") == 0)
        {
            save->size = atoi(value);
            save->header.datasize = ES32(save->size);
        }
        else if (strcmp(name, "date") == 0 || strcmp(name, "time") == 0)
        {
            char attr[16], val[16];
            struct xml_string *xmldate, *content;

            for (int j = 0; j < 3; j++)
            {
                xmldate = xml_node_attribute_name(item, j);
                content = xml_node_attribute_content(item, j);

                memset(val, 0, sizeof(val));
                memset(attr, 0, sizeof(attr));
                xml_string_copy(xmldate, (uint8_t*) attr, sizeof(attr));
                xml_string_copy(content, (uint8_t*) val, sizeof(val));

                if (strcmp(attr, "year") == 0)
                    date.year = atoi(val) - 1980;

                else if (strcmp(attr, "month") == 0)
                    date.month = atoi(val);

                else if (strcmp(attr, "day") == 0)
                    date.day = atoi(val);

                else if (strcmp(attr, "hour") == 0)
                    date.time = atoi(val);

                else if (strcmp(attr, "minute") == 0)
                    date.min = atoi(val);
            }
        }
        else if (strcmp(name, "data") == 0)
        {
            save->data = base64_decode((uint8_t*) value, strlen(value), &dlen);
            if (!save->data || dlen != save->size)
            {
                printf("[X] Error loading Base64 save data!\n");
                free(save);
                return NULL;
            }
        }

        free(name);
        free(value);
    }

    save->header.date = bup_setdate(&date);
    save->header.date = ES32(save->header.date);

    // Remember to free the document or you'll risk a memory leak
    xml_document_free(document, true);

    return save;
}

int check_CMS_save(const u8 *src_data, size_t len)
{
    return (len > sizeof(ss_header_block_t) && memcmp(src_data, "\x80\x0\x0\x0", 4) == 0);
}

SaturnSave *load_CMS_save(const u8 *src_data, size_t len)
{
    size_t i;
    SaturnSave *save;
    ss_header_block_t *header = (ss_header_block_t*)src_data;

    save = (SaturnSave*)malloc(sizeof(SaturnSave));
    memset(save, 0, sizeof(SaturnSave));
    memcpy(save->header.filename, header->filename, 11);
    memcpy(save->header.comment, header->comment, 10);
    save->header.language = header->language;
    save->header.datasize = header->size;
    save->header.date = header->date;
    save->size = ES32(header->size);

    // adjust data blocks
    for (i = 1; i < (len/0x40); i++)
    {
        memmove((void*)(src_data + 0x40 + 60*(i-1)), (void*)(src_data + i*0x40 + 4), 60);
    }

    // find data start
    for (i = 0x22; i < len; i += 2)
    {
        if (*(u16*)(src_data + i) == 0)
        {
            src_data += (i + 2);
            break;
        }
    }

    save->data = (u8*)malloc(save->size);
    memcpy(save->data, src_data, save->size);

    return save;
}

int check_SAROO_save(const u8 *src_data, size_t len)
{
    SAROOHeader *header = (SAROOHeader*)src_data;

    return (len > sizeof(SAROOHeader) && memcmp(header->magic, SAROO_MAGIC, 8) == 0);
}

SaturnSave *load_SAROO_save(const u8 *src_data, size_t len)
{
    SaturnSave *save;
    SAROOHeader *header = (SAROOHeader*)src_data;

    if (header->crc32 && ES32(header->crc32) != crc32b(src_data + 0x10, len - 0x10))
    {
        printf("[!] Warning: CRC32 mismatch!\n");
    }

    save = (SaturnSave*)malloc(sizeof(SaturnSave));
    memset(save, 0, sizeof(SaturnSave));
    memcpy(save->header.filename, header->filename, 11);
    memcpy(save->header.comment, header->comment, 10);
    save->header.datasize = header->size;
    save->header.date = header->date;
    save->header.language = header->language;
    save->size = ES32(header->size);

    save->data = (u8*)malloc(save->size);
    memcpy(save->data, src_data + sizeof(SAROOHeader), save->size);

    return save;
}

int export_SAROO(const SaturnSave* save)
{
    char fname[32];
    uint8_t *buf = malloc(sizeof(SAROOHeader) + save->size);
    SAROOHeader *header = (SAROOHeader*) buf;

    memset(header, 0, sizeof(SAROOHeader));
    memcpy(header->magic, SAROO_MAGIC, 8);
    memcpy(header->filename, save->header.filename, 11);
    memcpy(header->comment, save->header.comment, 10);
    header->size = save->header.datasize;
    header->date = save->header.date;
    header->language = save->header.language;

    memcpy(buf + sizeof(SAROOHeader), save->data, save->size);
    header->crc32 = crc32b(buf + 0x10, save->size + 0x30);
    header->crc32 = ES32(header->crc32);

    snprintf(fname, sizeof(fname), "%s%s", save->filename, ".SRO");
    if (write_buffer(fname, buf, save->size + sizeof(SAROOHeader)) < 0)
    {
        printf("[X] ERROR: Couldn't create file! (%s)\n", fname);
        return 0;
    }

    printf("[*] Exporting %s...\n    > Format  :\t SAROO save (SSAVERAW)\n", fname);
    printf("    > CRC32   :\t %08X\n", ES32(header->crc32));
    print_details(save);
    return (1);
}

int check_BUP_save(const u8 *src_data, size_t len)
{
    vmem_bup_header_t *header = (vmem_bup_header_t*)src_data;

    return (len > sizeof(vmem_bup_header_t) && memcmp(header->magic, VMEM_MAGIC_STRING, VMEM_MAGIC_STRING_LEN) == 0);
}

SaturnSave *load_BUP_save(const u8 *src_data, size_t len)
{
    SaturnSave *save;
    vmem_bup_header_t *header = (vmem_bup_header_t*)src_data;

    save = (SaturnSave*)malloc(sizeof(SaturnSave));
    memset(save, 0, sizeof(SaturnSave));
    memcpy(&save->header, &header->dir, sizeof(jo_backup_file));
    save->header.filename[11] = 0;
    save->header.comment[10] = 0;
    save->size = ES32(header->dir.datasize);

    save->data = (u8*)malloc(save->size);
    memcpy(save->data, src_data + sizeof(vmem_bup_header_t), save->size);

    return save;
}

int export_BUP(const SaturnSave* save)
{
    FILE *fp;
    char fname[32];
    vmem_bup_header_t header = {0};

    memcpy(header.magic, VMEM_MAGIC_STRING, VMEM_MAGIC_STRING_LEN);
    memcpy(&header.dir, &save->header, sizeof(jo_backup_file));
    header.dir.blocksize = calculateUsedBlocks(save->size, SAT_CLUSTER_SIZE);
    header.date = save->header.date;

    snprintf(fname, sizeof(fname), "%s%s", save->filename, BUP_EXTENSION);
    fp = fopen(fname, "wb");
    if (!fp)
    {
        printf("[X] ERROR: Couldn't create file! (%s)\n", fname);
        return 0;
    }

    printf("[*] Exporting %s...\n    > Format  :\t BUP (Pseudo Saturn Kai)\n", fname);
    fwrite(&header, sizeof(vmem_bup_header_t), 1, fp);
    fwrite(save->data, save->size, 1, fp);
    fclose(fp);

    print_details(save);
    return (1);
}

void print_usage(const char* argv0)
{
    printf("USAGE: %s filename [-s]\n\n", argv0);
    printf("Opt:\t-s\tConvert to SAROO single save format\n\n");
    return;
}

int main(int argc, char **argv)
{
    size_t len;
    uint8_t *data;
    SaturnSave *save = NULL;
    save_format_t formats[] = {
        {"SRO (SAROO single save)", check_SAROO_save, load_SAROO_save},
        {"BUP (Pseudo Saturn Kai)", check_BUP_save, load_BUP_save},
        {"XML (Backup RAM Parser)", check_XML_save, load_XML_save},
        {"B64 (Save Game Manager)", check_B64_save, load_B64_save},
        {"CMS (Comms Link)", check_CMS_save, load_CMS_save},
        {NULL, NULL}
    };

    printf("\nSEGA Saturn save converter 0.1.0 - (c) 2025 by Bucanero\n\n");

    if (--argc < 1)
    {
        print_usage(argv[0]);
        return -1;
    }

    if (read_buffer(argv[1], &data, &len) != 0)
    {
        printf("[*] Can't access the file (%s)\n", argv[1]);
        return -1;
    }

    printf("[*] Loading %s...\n", argv[1]);
    for(save_format_t* fmt = formats; fmt->formatChecker; fmt++)
    {
        if (fmt->formatChecker(data, len))
        {
            printf("    > Type:\t %s\n\n", fmt->type);
            save = fmt->saveLoader(data, len);
            break;
        }
    }

    // force SAROO export format if source is BUP
    if (check_BUP_save(data, len))
        argc = 0;

    free(data);
    if (!save)
    {
        printf("[X] ERROR: Can't detect save format!\n\n");
        return -1;
    }
    save->filename = strdup(argv[1]);
    strrchr(save->filename, '.')[0] = 0;

    if (!argc || (argc > 1 && strcmp(argv[2], "-s") == 0))
        len = export_SAROO(save);
    else
        len = export_BUP(save);

    if (len)
        printf("[i] Successfully exported save file.\n\n");
    else
        printf("[X] Error exporting %s!\n\n", argv[1]);

    free(save->data);
    free(save);

    return 0;
}
