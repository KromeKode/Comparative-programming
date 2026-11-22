// A const fn can run while compiling. Inside one, loops are written with
// while, because a for loop needs the Iterator trait, which isn't const.
const fn make_crc_table() -> [u32; 256] {
    let mut table = [0u32; 256];
    let mut i = 0;
    while i < 256 {
        let mut c = i as u32;
        let mut round = 0;
        while round < 8 {
            c = if c & 1 == 1 {
                0xEDB8_8320 ^ (c >> 1)
            } else {
                c >> 1
            };
            round += 1;
        }
        table[i] = c;
        i += 1;
    }
    table
}

// A const item is always worked out while compiling.
const CRC_TABLE: [u32; 256] = make_crc_table();

const fn crc32(bytes: &[u8]) -> u32 {
    let mut crc = u32::MAX;
    let mut i = 0;
    while i < bytes.len() {
        crc = CRC_TABLE[((crc ^ bytes[i] as u32) & 0xFF) as usize] ^ (crc >> 8);
        i += 1;
    }
    !crc
}

// An assert in a const block is checked while compiling, so a wrong table
// stops the build.
const _: () = assert!(CRC_TABLE[1] == 0x7707_3096);
const CHECK: u32 = crc32(b"123456789");
const _: () = assert!(CHECK == 0xCBF4_3926); // the standard check value

fn main() {
    println!("table[1] = {:08x}", CRC_TABLE[1]);
    println!("table[255] = {:08x}", CRC_TABLE[255]);
    println!("crc32(\"123456789\") = {CHECK:08x}");

    // The same function also runs as ordinary code, on text that only
    // exists when the program runs.
    for text in ["hello", "The quick brown fox jumps over the lazy dog", ""] {
        println!("crc32(\"{text}\") = {:08x}", crc32(text.as_bytes()));
    }
}
