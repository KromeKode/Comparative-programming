#include <stdint.h>
#include <stdio.h>

// CRC-32 runs each byte through a table of 256 numbers. Entry i is i after
// eight rounds of shifting right, each round also XOR-ing in the polynomial
// when the bit shifted out was 1.
#define POLY UINT32_C(0xEDB88320)
#define CRC_ROUND(c) (((c) >> 1) ^ (POLY & (UINT32_C(0) - ((c) & 1))))
#define CRC_ROUND_4(c) CRC_ROUND(CRC_ROUND(CRC_ROUND(CRC_ROUND(c))))
#define CRC_ENTRY(i) CRC_ROUND_4(CRC_ROUND_4((uint32_t)(i)))

// The preprocessor can't loop, so these macros paste out 256 entries,
// 4 at a time. Each entry is a constant expression for the compiler to work
// out, which makes the whole table a constant built into the program.
#define ENTRIES_4(i) CRC_ENTRY(i), CRC_ENTRY(i + 1), CRC_ENTRY(i + 2), CRC_ENTRY(i + 3)
#define ENTRIES_16(i) ENTRIES_4(i), ENTRIES_4(i + 4), ENTRIES_4(i + 8), ENTRIES_4(i + 12)
#define ENTRIES_64(i) ENTRIES_16(i), ENTRIES_16(i + 16), ENTRIES_16(i + 32), ENTRIES_16(i + 48)

static const uint32_t crc_table[256] = {
    ENTRIES_64(0), ENTRIES_64(64), ENTRIES_64(128), ENTRIES_64(192),
};

// _Static_assert checks a constant expression while compiling.
_Static_assert(CRC_ENTRY(1) == UINT32_C(0x77073096), "CRC table entry 1 is wrong");
_Static_assert(sizeof crc_table / sizeof crc_table[0] == 256, "the table needs 256 entries");

// C has no way to run this function while compiling, so it runs here, when
// the program does.
static uint32_t crc32(const char *text) {
    uint32_t crc = UINT32_MAX;
    for (const unsigned char *p = (const unsigned char *)text; *p; p++) {
        crc = crc_table[(crc ^ *p) & 0xFF] ^ (crc >> 8);
    }
    return ~crc;
}

int main(void) {
    printf("table[1] = %08x\n", (unsigned)crc_table[1]);
    printf("table[255] = %08x\n", (unsigned)crc_table[255]);
    const char *texts[] = {
        "123456789",
        "hello",
        "The quick brown fox jumps over the lazy dog",
        "",
    };
    for (size_t i = 0; i < sizeof texts / sizeof texts[0]; i++) {
        printf("crc32(\"%s\") = %08x\n", texts[i], (unsigned)crc32(texts[i]));
    }
    return 0;
}
