#include <stdint.h>

#define SAROO_MAGIC "SSAVERAW"

// SAROO Single Save format
typedef struct
{
    char magic[8];
    char pad1[4];
    uint32_t crc32;
    char filename[12];
    uint32_t size;
    char comment[11];
    char language;
    uint32_t date;
    char pad2[16];
} __attribute__((packed)) SAROOHeader;

/*
Save Game Manager (SGM) savegame format by Rockin'-B
http://www.rockin-b.de/saturn-savegamemanager.html
Thomas Fuchs / The Rockin'-B, www.rockin-b.de
*/
typedef struct MEMORY_FILE {
    char name[12+1]; // CD filenames must fit in, too
    char comment[10+1];
    uint8_t language, minute, hour, weekday, day, month;
    uint16_t year;
    uint32_t size;
    char dummy[4];
} __attribute__((packed)) MemoryFile;

typedef struct {
    MemoryFile header;
    uint32_t size;
    uint8_t *data;
} SGMFile;

// Saturn block header
typedef struct {
    uint32_t magic;             // 0x00: 80000000
    char filename[11];          // 0x04: Archive name, 11 bytes.
    char language;              // 0x0f: Language flag.
    char comment[10];           // 0x10: Archive comment, 10 bytes.
    uint32_t date;              // 0x1a: Archive date encoding, 4 bytes.
    uint32_t size;              // 0x1e: Archive byte size, 4 bytes. (big-endian)
} __attribute__((packed)) ss_header_block_t;
/*
 0x22: List of blocks occupied by the archive. Ends with 0000.
       When the starting block is used up, it continues to the next block.
       The first 4 bytes of the block need to be skipped.
*/
