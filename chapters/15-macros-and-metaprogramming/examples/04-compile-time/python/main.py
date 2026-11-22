def make_crc_table() -> list[int]:
    table = []
    for i in range(256):
        c = i
        for _ in range(8):
            c = 0xEDB88320 ^ (c >> 1) if c & 1 else c >> 1
        table.append(c)
    return table


# Python has no compile-time evaluation. Code at the top of a module runs
# when the module is imported, so the table is built each time the program
# starts, before main runs.
CRC_TABLE = make_crc_table()


def crc32(data: bytes) -> int:
    crc = 0xFFFFFFFF
    for byte in data:
        crc = CRC_TABLE[(crc ^ byte) & 0xFF] ^ (crc >> 8)
    return crc ^ 0xFFFFFFFF


# A check at import time is the nearest thing to a static_assert.
assert crc32(b"123456789") == 0xCBF43926, "the CRC table is wrong"


def main() -> None:
    print(f"table[1] = {CRC_TABLE[1]:08x}")
    print(f"table[255] = {CRC_TABLE[255]:08x}")
    for text in [
        "123456789",
        "hello",
        "The quick brown fox jumps over the lazy dog",
        "",
    ]:
        print(f'crc32("{text}") = {crc32(text.encode()):08x}')


if __name__ == "__main__":
    main()
