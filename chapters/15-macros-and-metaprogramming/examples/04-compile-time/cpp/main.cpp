#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

// A constexpr function can run while compiling, loops and all, when its
// result is needed as a constant.
constexpr std::array<std::uint32_t, 256> make_crc_table() {
    std::array<std::uint32_t, 256> table{};
    for (std::uint32_t i = 0; i < 256; i++) {
        std::uint32_t c = i;
        for (int round = 0; round < 8; round++) {
            c = (c & 1) ? 0xEDB88320 ^ (c >> 1) : c >> 1;
        }
        table[i] = c;
    }
    return table;
}

// constexpr on a variable means it must be worked out while compiling, so
// the table is built into the program, and nothing builds it at run time.
constexpr auto crc_table = make_crc_table();

constexpr std::uint32_t crc32(std::string_view text) {
    std::uint32_t crc = 0xFFFFFFFF;
    for (unsigned char byte : text) crc = crc_table[(crc ^ byte) & 0xFF] ^ (crc >> 8);
    return ~crc;
}

// static_assert stops the build if the condition is false. The compiler
// runs crc32 to find out, so a wrong table can never be compiled.
static_assert(crc_table[1] == 0x77073096);
static_assert(crc32("123456789") == 0xCBF43926);  // the standard check value

// consteval goes further: every call must be worked out while compiling.
consteval std::uint32_t crc32_now(std::string_view text) { return crc32(text); }

int main() {
    std::printf("table[1] = %08x\n", crc_table[1]);
    std::printf("table[255] = %08x\n", crc_table[255]);
    constexpr std::uint32_t check = crc32_now("123456789");
    std::printf("crc32(\"123456789\") = %08x\n", check);

    // The same function also runs as ordinary code, on text that only
    // exists when the program runs.
    for (std::string_view text : {"hello", "The quick brown fox jumps over the lazy dog", ""}) {
        std::printf("crc32(\"%.*s\") = %08x\n", static_cast<int>(text.size()), text.data(),
                    crc32(text));
    }
}
