# Exercise 2.2: Byte casts

Each line of input holds one whole number `x`, with −2^31 ≤ x < 2^31. Print what `x`
becomes when you keep only its lowest 8 bits, read first as an unsigned byte (0 to
255) and then as a signed byte (−128 to 127):

```text
<x> -> u8 <unsigned value>, i8 <signed value>
```

Skip blank lines.

| Input | Output |
| --- | --- |
| `200` | `200 -> u8 200, i8 -56` |
| `300` | `300 -> u8 44, i8 44` |
| `-1` | `-1 -> u8 255, i8 -1` |

**How it works:** 300 in binary is `1 0010 1100`. The lowest 8 bits are `0010 1100`,
which is 44. For 200 (`1100 1000`), the top bit is 1, so as a signed byte it means
200 − 256 = −56. This is **two's complement**, the way every modern CPU stores signed
integers.

```sh
python3 tools/check.py 02 --lang cpp
```

**Things to notice:** Rust, Go, C++20, and Kotlin do this with a plain conversion, such
as `x as u8`, `uint8(x)`, `static_cast<std::uint8_t>(x)`, or `x.toUByte()`. Python,
JavaScript, and Dart have no 8-bit type, so you use bit operations or a method instead. In C++, try printing
a `uint8_t` directly with `std::cout` and see what happens.
