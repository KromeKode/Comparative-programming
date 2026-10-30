# Chapter 6: Strings and text

Every program handles text, and text is where the nine languages differ most under a
familiar surface. A reader sees five characters in `"Zoë 👍🏽"`. Ask for its length and
C, C++, Rust, and Go say 13, Python says 6, and JavaScript, TypeScript, Dart, and Kotlin
say 8. None of them is wrong. They are counting different things: bytes, code points,
and UTF-16 code units. This chapter explains what each language counts and why, then
covers indexing, building strings, the everyday operations (trim, split, search,
replace, and change case), comparing and sorting, and writing string literals.

**Languages:** all 9 · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| Bytes, code points, and grapheme clusters; UTF-8 and UTF-16 | Files and the command line (chapter 20), text parsing (chapter 21), web servers (chapter 36) |
| Each language's string types, and which ones can change | The memory model and ownership (chapters 9 and 10) |
| Measuring, indexing, and slicing without cutting a character in half | Text parsing (chapter 21), arrays and strings (chapter 24) |
| Building strings without copying them over and over | Complexity and benchmarking (chapter 23) |
| Formatting numbers and padding columns | Command-line tools (chapter 20) |
| Searching, splitting, replacing, and changing case | Text parsing (chapter 21) |
| Comparing and sorting text | Sorting and searching (chapter 28) |

---

## 6.1 The concept: text is numbers, in layers

A computer stores text as numbers. There are three layers to keep apart.

1. A **code point** is the number Unicode gives a character. U+0041 is `A`, U+00EB
   is `ë`, and U+1F44D is 👍. Code points run from U+0000 to U+10FFFF.
2. An **encoding** turns each code point into **code units** of a fixed size. UTF-8
   uses one to four 8-bit units (bytes) per code point. UTF-16 uses one or two 16-bit
   units. UTF-32 uses one 32-bit unit for every code point.
3. A **grapheme cluster** is what a reader calls one character. It can take several
   code points. 👍🏽 is 👍 followed by U+1F3FD, a skin-tone modifier. And `ë` can be
   stored as one code point, U+00EB, or as `e` followed by U+0308, a combining
   diaeresis.

Here is `"Zoë 👍🏽"`, the string this chapter uses throughout, in each layer:

| Grapheme | Code points | UTF-8 bytes | UTF-16 code units |
| --- | --- | --- | --- |
| `Z` | U+005A | `5A` | `005A` |
| `o` | U+006F | `6F` | `006F` |
| `ë` | U+00EB | `C3 AB` | `00EB` |
| space | U+0020 | `20` | `0020` |
| 👍🏽 | U+1F44D U+1F3FD | `F0 9F 91 8D F0 9F 8F BD` | `D83D DC4D D83C DFFD` |
| **5** | **6** | **13** | **8** |

**UTF-8** is the encoding of the web, of most files, and of file names on Linux and
macOS. The top bits of each byte say what the byte is:

| Code points | Bytes | Bit pattern |
| --- | --- | --- |
| U+0000 to U+007F | 1 | `0xxxxxxx` |
| U+0080 to U+07FF | 2 | `110xxxxx 10xxxxxx` |
| U+0800 to U+FFFF | 3 | `1110xxxx 10xxxxxx 10xxxxxx` |
| U+10000 to U+10FFFF | 4 | `11110xxx 10xxxxxx 10xxxxxx 10xxxxxx` |

That design has four useful properties:

- ASCII text is already UTF-8, byte for byte.
- Every byte of the form `10xxxxxx` continues a character, and every other byte
  starts one. So you can count code points by counting the bytes that do not start
  with `10`, which is what the C and C++ examples below do.
- A byte below 128 never appears inside a longer sequence, so a search for `,` or
  `\n` in UTF-8 bytes never finds half a character.
- Sorting UTF-8 strings byte by byte gives the same order as sorting by code point.

**UTF-16** stores every code point up to U+FFFF as one 16-bit unit, and the rest as a
**surrogate pair**: two units from the range D800 to DFFF, which Unicode keeps free for
this purpose. Java, JavaScript, and Windows chose 16-bit characters when Unicode was
expected to fit in 16 bits, and Dart and Kotlin inherited that choice from JavaScript
and Java (section 6.9).

**Normalization.** Because `ë` can be one code point or two, two strings can look the
same and still compare unequal. Unicode defines **normal forms** that pick one
spelling. NFC composes (`e` followed by U+0308 becomes U+00EB) and NFD decomposes.
Section 6.7 shows which languages can normalize.

---

## 6.2 String types

| Language | Text type | Holds | Can change in place | Read-only view | Raw bytes |
| --- | --- | --- | --- | --- | --- |
| C | `char[]` or `char *`, ending at a `'\0'` byte | Bytes in any encoding; UTF-8 by convention on Linux and macOS | Arrays yes, string literals no | `const char *`, plus a length if it is not the whole string | `unsigned char[]` |
| C++ | `std::string` | Bytes, with a stored length | Yes | `std::string_view` (C++17) | `std::string`, or `std::vector<std::byte>` |
| Rust | `String` (owned) and `&str` (borrowed) | UTF-8, always valid | A `String` declared `mut` | `&str` | `Vec<u8>` and `&[u8]` |
| Go | `string` | Bytes, by convention UTF-8 | No | Slicing a `string` shares its bytes | `[]byte` |
| Python | `str` | Code points | No | None; slicing copies | `bytes`, and `bytearray`, which can change |
| JavaScript | `string` | UTF-16 code units | No | None | `Uint8Array` |
| TypeScript | `string` | UTF-16 code units | No | None | `Uint8Array` |
| Dart | `String` | UTF-16 code units | No | None | `Uint8List`, or `List<int>` in many APIs |
| Kotlin | `String` | UTF-16 code units | No | `CharSequence`, an interface that `String` and `StringBuilder` share | `ByteArray` |

**C's terminator.** A C string has no stored length. `strlen` walks the bytes until it
finds a zero, so it takes time proportional to the length, and a C string cannot
contain a zero byte. Every other language here stores the length, and its length
operation is instant.

**How Python stores code points.** CPython keeps each string at 1, 2, or 4 bytes per
code point, whichever its largest code point needs. An ASCII string costs one byte per
character, and `s[i]` is still instant. The JVM does something similar: since JDK 9, a
string that fits in Latin-1 is stored with one byte per character, but Kotlin's API
still counts in UTF-16 units.

**Immutable strings.** In Go, Python, JavaScript, TypeScript, Dart, and Kotlin, a
string never changes after it is made. `upper()` and `replace()` return a new string.
That makes a string safe to share between variables, threads, and map keys, and lets
the runtime keep one copy of each literal. Immutability belongs to the string, not the
variable: `s += "!"` works on any variable you may reassign. It makes a new string and
points `s` at it.

Here is what happens when you try to change one character of `"Zoë"`:

| Language | `s[0] = 'z'` |
| --- | --- |
| C | Works on `char s[] = "Zoë"`, which copies the literal into an array. Through `char *p = "Zoë"`, it compiles with no warning under `-Wall -Wextra`, and the behavior is undefined: string literals live in read-only memory, and on Linux the program dies with a segmentation fault |
| C++ | Works on a `std::string`, giving `zoë`. `char *p = "Zoë"` gets a warning, `ISO C++ forbids converting a string constant to 'char*'`, which this book's `-Werror` turns into an error |
| Rust | E0277: `the type 'str' cannot be indexed by '{integer}'`. Change a `String` with `push`, `insert`, or `replace_range`, which need `mut` (E0596 without it: `cannot borrow 's' as mutable, as it is not declared as mutable`) |
| Go | Compile error: `cannot assign to s[0] (neither addressable nor a map index expression)` |
| Python | `TypeError: 'str' object does not support item assignment` |
| JavaScript | In strict mode, and in every module: `TypeError: Cannot assign to read only property '0' of string 'Zoë'`. In old-style scripts the assignment is silently ignored |
| TypeScript | TS2542: `Index signature in type 'String' only permits reading.` |
| Dart | `The operator '[]=' isn't defined for the type 'String'.` |
| Kotlin | `no 'set' operator method providing array access.` |

To change many characters in a language with immutable strings, convert to something
that can change, edit it, and convert back: a `[]byte` or `[]rune` in Go, a `list` in
Python, an array in JavaScript, or a `StringBuilder` in Kotlin.

---

## 6.3 Measuring a string

The program below prints `"Zoë 👍🏽"`, the length the language reports, and then the
number of UTF-8 bytes, UTF-16 code units, and code points. JavaScript, TypeScript, and
Kotlin also count grapheme clusters, because their standard libraries can. The code is
in [`examples/01-length/`](examples/01-length/).

**C**

```c
#include <stdio.h>
#include <string.h>

// In UTF-8, every byte of the form 10xxxxxx continues a code point, and every other
// byte starts one. So counting the bytes that are not 10xxxxxx counts code points.
static size_t count_code_points(const char *s) {
    size_t count = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p != '\0'; p++) {
        if ((*p & 0xC0) != 0x80) {
            count++;
        }
    }
    return count;
}

// A code point above U+FFFF needs 4 bytes in UTF-8 (first byte 11110xxx) and two
// code units in UTF-16. Everything else needs one UTF-16 code unit.
static size_t count_utf16_units(const char *s) {
    size_t count = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p != '\0'; p++) {
        if ((*p & 0xC0) != 0x80) {
            count += *p >= 0xF0 ? 2 : 1;
        }
    }
    return count;
}

int main(void) {
    // \u and \U name a code point; GCC and Clang store the string as UTF-8.
    const char *s = "Zo\u00eb \U0001F44D\U0001F3FD";

    printf("text: %s\n", s);
    printf("built-in length: %zu (bytes)\n", strlen(s));
    printf("UTF-8 bytes: %zu\n", strlen(s));
    printf("UTF-16 code units: %zu\n", count_utf16_units(s));
    printf("code points: %zu\n", count_code_points(s));
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <string>
#include <string_view>

// A byte of the form 10xxxxxx continues a code point; every other byte starts one.
std::size_t count_code_points(std::string_view s) {
    std::size_t count = 0;
    for (unsigned char byte : s) {
        if ((byte & 0xC0) != 0x80) {
            ++count;
        }
    }
    return count;
}

// Code points above U+FFFF start with a byte 11110xxx and need two UTF-16 code units.
std::size_t count_utf16_units(std::string_view s) {
    std::size_t count = 0;
    for (unsigned char byte : s) {
        if ((byte & 0xC0) != 0x80) {
            count += byte >= 0xF0 ? 2 : 1;
        }
    }
    return count;
}

int main() {
    // std::string holds bytes. GCC and Clang store this literal as UTF-8.
    const std::string s = "Zo\u00eb \U0001F44D\U0001F3FD";

    std::cout << "text: " << s << '\n';
    std::cout << "built-in length: " << s.size() << " (bytes)\n";
    std::cout << "UTF-8 bytes: " << s.size() << '\n';
    std::cout << "UTF-16 code units: " << count_utf16_units(s) << '\n';
    std::cout << "code points: " << count_code_points(s) << '\n';
}
```

**Rust**

```rust
fn main() {
    // A Rust string is always valid UTF-8. \u{...} names a code point.
    let s = "Zo\u{eb} \u{1F44D}\u{1F3FD}";

    println!("text: {s}");
    println!("built-in length: {} (bytes)", s.len());
    println!("UTF-8 bytes: {}", s.len());
    println!("UTF-16 code units: {}", s.encode_utf16().count());
    println!("code points: {}", s.chars().count()); // a char is one code point
}
```

**Go**

```go
package main

import (
	"fmt"
	"unicode/utf16"
	"unicode/utf8"
)

func main() {
	// A Go string is a read-only slice of bytes, by convention UTF-8.
	s := "Zo\u00eb \U0001F44D\U0001F3FD"

	fmt.Println("text:", s)
	fmt.Printf("built-in length: %d (bytes)\n", len(s))
	fmt.Println("UTF-8 bytes:", len(s))
	fmt.Println("UTF-16 code units:", len(utf16.Encode([]rune(s))))
	fmt.Println("code points:", utf8.RuneCountInString(s)) // a rune is one code point
}
```

**Python**

```python
def main() -> None:
    # A Python str is a sequence of code points; encode() turns it into bytes.
    s = "Zo\u00eb \U0001f44d\U0001f3fd"

    print("text:", s)
    print(f"built-in length: {len(s)} (code points)")
    print("UTF-8 bytes:", len(s.encode("utf-8")))
    print("UTF-16 code units:", len(s.encode("utf-16-le")) // 2)  # 2 bytes per unit
    print("code points:", len(s))


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
// A JavaScript string is a sequence of UTF-16 code units.
const s = "Zo\u00eb \u{1F44D}\u{1F3FD}";

console.log("text:", s);
console.log(`built-in length: ${s.length} (UTF-16 code units)`);
console.log("UTF-8 bytes:", new TextEncoder().encode(s).length);
console.log("UTF-16 code units:", s.length);
console.log("code points:", [...s].length); // spreading a string splits it by code point

// Intl.Segmenter finds the characters a reader sees: grapheme clusters.
const segmenter = new Intl.Segmenter("en", { granularity: "grapheme" });
console.log("graphemes:", [...segmenter.segment(s)].length);
```

**TypeScript**

```ts
// A TypeScript string is a JavaScript string: a sequence of UTF-16 code units.
const s: string = "Zo\u00eb \u{1F44D}\u{1F3FD}";

console.log("text:", s);
console.log(`built-in length: ${s.length} (UTF-16 code units)`);
console.log("UTF-8 bytes:", new TextEncoder().encode(s).length);
console.log("UTF-16 code units:", s.length);
console.log("code points:", [...s].length); // spreading a string splits it by code point

// Intl.Segmenter finds the characters a reader sees: grapheme clusters.
const segmenter = new Intl.Segmenter("en", { granularity: "grapheme" });
console.log("graphemes:", [...segmenter.segment(s)].length);

export {};
```

**Dart**

```dart
import 'dart:convert';

void main() {
  // A Dart String is a sequence of UTF-16 code units.
  const s = 'Zo\u00eb \u{1F44D}\u{1F3FD}';

  print('text: $s');
  print('built-in length: ${s.length} (UTF-16 code units)');
  print('UTF-8 bytes: ${utf8.encode(s).length}');
  print('UTF-16 code units: ${s.codeUnits.length}');
  print('code points: ${s.runes.length}'); // runes are code points
}
```

**Kotlin**

```kotlin
import java.text.BreakIterator

fun main() {
    // A Kotlin String is a sequence of UTF-16 code units. \u takes exactly four hex
    // digits, so U+1F44D and U+1F3FD are written as their two UTF-16 halves.
    val s = "Zo\u00eb \uD83D\uDC4D\uD83C\uDFFD"

    println("text: $s")
    println("built-in length: ${s.length} (UTF-16 code units)")
    println("UTF-8 bytes: ${s.toByteArray().size}") // toByteArray() encodes as UTF-8
    println("UTF-16 code units: ${s.length}")
    println("code points: ${s.codePointCount(0, s.length)}")

    // Java's BreakIterator finds grapheme clusters, the characters a reader sees.
    val boundaries = BreakIterator.getCharacterInstance()
    boundaries.setText(s)
    var graphemes = 0
    while (boundaries.next() != BreakIterator.DONE) graphemes++
    println("graphemes: $graphemes")
}
```

Output:

*C, C++, Rust and Go:*

```text
text: Zoë 👍🏽
built-in length: 13 (bytes)
UTF-8 bytes: 13
UTF-16 code units: 8
code points: 6
```

*Python:*

```text
text: Zoë 👍🏽
built-in length: 6 (code points)
UTF-8 bytes: 13
UTF-16 code units: 8
code points: 6
```

*JavaScript, TypeScript and Kotlin:*

```text
text: Zoë 👍🏽
built-in length: 8 (UTF-16 code units)
UTF-8 bytes: 13
UTF-16 code units: 8
code points: 6
graphemes: 5
```

*Dart:*

```text
text: Zoë 👍🏽
built-in length: 8 (UTF-16 code units)
UTF-8 bytes: 13
UTF-16 code units: 8
code points: 6
```

| Language | Built-in length | Counts | Code points | Grapheme clusters |
| --- | --- | --- | --- | --- |
| C | `strlen(s)` | Bytes before the first `'\0'` | Count the bytes not of the form `10xxxxxx` | No |
| C++ | `s.size()` | Bytes | The same loop as C | No |
| Rust | `s.len()` | Bytes | `s.chars().count()` | The `unicode-segmentation` crate |
| Go | `len(s)` | Bytes | `utf8.RuneCountInString(s)` | Third-party modules, such as `github.com/rivo/uniseg` |
| Python | `len(s)` | Code points | `len(s)` | The third-party `regex` module's `\X` |
| JavaScript, TypeScript | `s.length` | UTF-16 code units | `[...s].length` | `Intl.Segmenter` |
| Dart | `s.length` | UTF-16 code units | `s.runes.length` | The `characters` package, from the Dart team |
| Kotlin | `s.length` | UTF-16 code units | `s.codePointCount(0, s.length)` | `java.text.BreakIterator` |

Counting code points or graphemes means walking the string in every language except
Python, whose `len` already counts code points.

**Which count to use.** Each one answers a different question:

- **Bytes** for buffer sizes, file sizes, network protocols, and database columns
  limited in bytes.
- **UTF-16 code units** for positions that JavaScript or Java code will use, such as
  offsets in a text editor.
- **Code points** for rules written in Unicode terms.
- **Grapheme clusters** for anything a person sees: moving a cursor, a form that allows
  20 characters, or cutting a long name and adding "…".

Cutting text by bytes or UTF-16 units can split a character in two. Cutting by code
points cannot, but it can still split 👍🏽 into 👍 and a lone skin-tone swatch.

---

## 6.4 Indexing and slicing

The program below prints the UTF-8 bytes of `"Zoë 👍🏽"` in hex, then each code point,
then the first three code points as a string. C and C++ decode UTF-8 by hand with the
bit patterns from section 6.1. The code is in
[`examples/02-code-points/`](examples/02-code-points/).

**C**

```c
#include <stdio.h>
#include <string.h>

// Decodes the code point that starts at p, stores it in *code_point, and returns how
// many bytes it used. It assumes valid UTF-8; chapter 36 shows how to check.
static int decode_utf8(const unsigned char *p, unsigned long *code_point) {
    if (p[0] < 0x80) {  // 0xxxxxxx: one byte, plain ASCII
        *code_point = p[0];
        return 1;
    }
    if (p[0] < 0xE0) {  // 110xxxxx 10xxxxxx
        *code_point = ((p[0] & 0x1Ful) << 6) | (p[1] & 0x3Ful);
        return 2;
    }
    if (p[0] < 0xF0) {  // 1110xxxx 10xxxxxx 10xxxxxx
        *code_point = ((p[0] & 0x0Ful) << 12) | ((p[1] & 0x3Ful) << 6) | (p[2] & 0x3Ful);
        return 3;
    }
    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    *code_point = ((p[0] & 0x07ul) << 18) | ((p[1] & 0x3Ful) << 12) |
                  ((p[2] & 0x3Ful) << 6) | (p[3] & 0x3Ful);
    return 4;
}

int main(void) {
    const char *s = "Zo\u00eb \U0001F44D\U0001F3FD";
    const unsigned char *bytes = (const unsigned char *)s;
    size_t length = strlen(s);

    printf("UTF-8 bytes:");
    for (size_t i = 0; i < length; i++) {
        printf(" %02X", bytes[i]);
    }
    printf("\n");

    printf("code points:");
    for (size_t i = 0; i < length;) {
        unsigned long code_point;
        i += (size_t)decode_utf8(bytes + i, &code_point);
        printf(" U+%04lX", code_point);
    }
    printf("\n");

    // Find where the third code point ends, then print that many bytes.
    size_t end = 0;
    for (int n = 0; n < 3; n++) {
        unsigned long ignored;
        end += (size_t)decode_utf8(bytes + end, &ignored);
    }
    printf("first 3 code points: %.*s\n", (int)end, s);
    return 0;
}
```

**C++**

```cpp
#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <string_view>

// Decodes the code point that starts at s[i] and moves i past it.
// It assumes valid UTF-8; chapter 36 shows how to check.
char32_t decode_utf8(std::string_view s, std::size_t& i) {
    auto byte = [&](std::size_t k) -> char32_t { return static_cast<unsigned char>(s[k]); };
    char32_t first = byte(i);
    if (first < 0x80) {  // 0xxxxxxx: one byte, plain ASCII
        i += 1;
        return first;
    }
    if (first < 0xE0) {  // 110xxxxx 10xxxxxx
        char32_t cp = ((first & 0x1F) << 6) | (byte(i + 1) & 0x3F);
        i += 2;
        return cp;
    }
    if (first < 0xF0) {  // 1110xxxx 10xxxxxx 10xxxxxx
        char32_t cp = ((first & 0x0F) << 12) | ((byte(i + 1) & 0x3F) << 6) | (byte(i + 2) & 0x3F);
        i += 3;
        return cp;
    }
    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    char32_t cp = ((first & 0x07) << 18) | ((byte(i + 1) & 0x3F) << 12) |
                  ((byte(i + 2) & 0x3F) << 6) | (byte(i + 3) & 0x3F);
    i += 4;
    return cp;
}

int main() {
    const std::string s = "Zo\u00eb \U0001F44D\U0001F3FD";

    std::cout << "UTF-8 bytes:";
    for (unsigned char b : s) {
        std::cout << std::format(" {:02X}", b);
    }
    std::cout << '\n';

    std::cout << "code points:";
    for (std::size_t i = 0; i < s.size();) {
        std::cout << std::format(" U+{:04X}", static_cast<std::uint32_t>(decode_utf8(s, i)));
    }
    std::cout << '\n';

    // substr counts bytes, so find the byte where the third code point ends.
    std::size_t end = 0;
    for (int n = 0; n < 3; ++n) {
        decode_utf8(s, end);
    }
    std::cout << "first 3 code points: " << s.substr(0, end) << '\n';
}
```

**Rust**

```rust
fn main() {
    let s = "Zo\u{eb} \u{1F44D}\u{1F3FD}";

    let bytes: Vec<String> = s.bytes().map(|b| format!("{b:02X}")).collect();
    println!("UTF-8 bytes: {}", bytes.join(" "));

    let code_points: Vec<String> = s.chars().map(|c| format!("U+{:04X}", c as u32)).collect();
    println!("code points: {}", code_points.join(" "));

    // &s[..3] would panic: byte 3 is in the middle of 'ë'. Count chars instead.
    let first_three: String = s.chars().take(3).collect();
    println!("first 3 code points: {first_three}");
}
```

**Go**

```go
package main

import (
	"fmt"
	"strings"
)

func main() {
	s := "Zo\u00eb \U0001F44D\U0001F3FD"

	var bytes strings.Builder
	for i := 0; i < len(s); i++ { // indexing a string gives bytes
		fmt.Fprintf(&bytes, " %02X", s[i])
	}
	fmt.Println("UTF-8 bytes:" + bytes.String())

	var codePoints strings.Builder
	for _, r := range s { // ranging over a string decodes runes
		fmt.Fprintf(&codePoints, " U+%04X", r)
	}
	fmt.Println("code points:" + codePoints.String())

	// s[:3] would cut 'ë' in half. Convert to runes first.
	fmt.Println("first 3 code points:", string([]rune(s)[:3]))
}
```

**Python**

```python
def main() -> None:
    s = "Zo\u00eb \U0001f44d\U0001f3fd"

    print("UTF-8 bytes:", s.encode("utf-8").hex(" ").upper())
    print("code points:", " ".join(f"U+{ord(c):04X}" for c in s))
    print("first 3 code points:", s[:3])  # indexes count code points


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
const s = "Zo\u00eb \u{1F44D}\u{1F3FD}";
const hex = (n, width) =>
  n.toString(16).toUpperCase().padStart(width, "0");

const bytes = [...new TextEncoder().encode(s)].map((b) => hex(b, 2));
console.log("UTF-8 bytes:", bytes.join(" "));

// for...of and [...s] walk code points; s[i] and s.length count UTF-16 code units.
const codePoints = [...s].map((c) => "U+" + hex(c.codePointAt(0), 4));
console.log("code points:", codePoints.join(" "));

console.log("first 3 code points:", [...s].slice(0, 3).join(""));
```

**TypeScript**

```ts
const s = "Zo\u00eb \u{1F44D}\u{1F3FD}";
const hex = (n: number, width: number): string =>
  n.toString(16).toUpperCase().padStart(width, "0");

const bytes = [...new TextEncoder().encode(s)].map((b) => hex(b, 2));
console.log("UTF-8 bytes:", bytes.join(" "));

// for...of and [...s] walk code points; s[i] and s.length count UTF-16 code units.
// codePointAt returns undefined past the end, so the type is number | undefined.
const codePoints = [...s].map((c) => "U+" + hex(c.codePointAt(0)!, 4));
console.log("code points:", codePoints.join(" "));

console.log("first 3 code points:", [...s].slice(0, 3).join(""));

export {};
```

**Dart**

```dart
import 'dart:convert';

String hex(int n, int width) =>
    n.toRadixString(16).toUpperCase().padLeft(width, '0');

void main() {
  const s = 'Zo\u00eb \u{1F44D}\u{1F3FD}';

  final bytes = utf8.encode(s).map((b) => hex(b, 2));
  print('UTF-8 bytes: ${bytes.join(' ')}');

  // s.runes walks code points; s[i] and s.length count UTF-16 code units.
  final codePoints = s.runes.map((r) => 'U+${hex(r, 4)}');
  print('code points: ${codePoints.join(' ')}');

  print('first 3 code points: ${String.fromCharCodes(s.runes.take(3))}');
}
```

**Kotlin**

```kotlin
fun main() {
    val s = "Zo\u00eb \uD83D\uDC4D\uD83C\uDFFD"

    val bytes = s.toByteArray().joinToString(" ") { "%02X".format(it) }
    println("UTF-8 bytes: $bytes")

    // codePoints() walks code points; s[i] and s.length count UTF-16 code units.
    val codePoints = s.codePoints().toArray().joinToString(" ") { "U+%04X".format(it) }
    println("code points: $codePoints")

    // substring takes UTF-16 positions, so find where the third code point ends.
    println("first 3 code points: ${s.substring(0, s.offsetByCodePoints(0, 3))}")
}
```

Output, identical in all nine:

```text
UTF-8 bytes: 5A 6F C3 AB 20 F0 9F 91 8D F0 9F 8F BD
code points: U+005A U+006F U+00EB U+0020 U+1F44D U+1F3FD
first 3 code points: Zoë
```

### What `s[i]` gives you

| Language | `s[2]` on `"Zoë"` | Position counts | `s[10]` |
| --- | --- | --- | --- |
| C | The byte `0xC3`, the first half of `ë` | Bytes | Undefined behavior |
| C++ | The same; `s.at(2)` also checks the position | Bytes | `s[10]` is undefined; `s.at(10)` throws `std::out_of_range` |
| Rust | Compile error (above). `s.as_bytes()[2]` is `0xC3`, and `s.chars().nth(2)` is `Some('ë')` | Bytes, for slices | `chars().nth(10)` is `None` |
| Go | `195` (`0xC3`), a `byte` | Bytes | Panic: `index out of range [10] with length 4` |
| Python | `'ë'`, a `str` of length 1 | Code points | `IndexError: string index out of range` |
| JavaScript, TypeScript | `'ë'`, a string of one UTF-16 unit | UTF-16 units | `undefined` |
| Dart | `'ë'`, a `String` of one UTF-16 unit | UTF-16 units | `RangeError (index): Invalid value: Not in inclusive range 0..2: 10` |
| Kotlin | `'ë'`, a `Char` | UTF-16 units | `StringIndexOutOfBoundsException: Index 10 out of bounds for length 3` |

`ë` is a single UTF-16 unit, so the UTF-16 languages return all of it. 👍 is two
units, and `"👍"[0]` in JavaScript, Dart, or Kotlin is half of a surrogate pair, which
is not a character at all.

Rust refuses `s[i]` because no answer is both fast and right. Returning the byte would
hand you half of `ë`, and finding the i-th code point means walking the string. Rust
makes you say which one you mean.

### Slicing

| Language | Characters 1 to 2 of `"Zoë"` | Positions count | Cutting inside a character |
| --- | --- | --- | --- |
| C | A pointer and a length: `printf("%.*s", 3, s + 1)` | Bytes | Allowed |
| C++ | `s.substr(1, 3)` copies; `std::string_view(s).substr(1, 3)` does not | Bytes | Allowed |
| Rust | `&s[1..4]` | Bytes | **Panics**: `&s[1..3]` stops with `end byte index 3 is not a char boundary; it is inside 'ë' (bytes 2..4 of string)`. `s.get(1..3)` returns `None` instead |
| Go | `s[1:4]` shares the bytes | Bytes | Allowed: `"Zoë"[:3]` is `"Zo\xc3"` |
| Python | `s[1:3]` | Code points | Not possible for code points; graphemes can still split |
| JavaScript, TypeScript | `s.slice(1, 3)` | UTF-16 units | Allowed: `"Zoë 👍".slice(0, 5)` ends with half of 👍 |
| Dart | `s.substring(1, 3)` | UTF-16 units | Allowed |
| Kotlin | `s.substring(1, 3)` | UTF-16 units | Allowed |

Python's slices and JavaScript's `slice` stop at the end of the string, so
`"Zoë"[1:100]` is `"oë"`. C++'s `substr` shortens a count that runs past the end, but
throws `std::out_of_range` for a start past it. Rust panics (`end byte index 100 is out
of bounds for string of length 4`), Go panics, Dart and Kotlin throw, and C reads past
the end.

### Walking code points

| Language | Each code point | With its position |
| --- | --- | --- |
| C, C++ | Decode by hand, as in the example | The decoder's byte position |
| Rust | `for c in s.chars()` | `for (i, c) in s.char_indices()`, `i` in bytes |
| Go | `for _, r := range s` | `for i, r := range s`, `i` in bytes |
| Python | `for c in s` | `for i, c in enumerate(s)` |
| JavaScript, TypeScript | `for (const c of s)`, or `[...s]` | `s.codePointAt(i)` reads the code point at unit `i` |
| Dart | `for (final r in s.runes)` | None built in |
| Kotlin | `s.codePoints()` | `s.codePointAt(i)` and `s.offsetByCodePoints(i, n)` |

Go's `range` turns each invalid byte into U+FFFD, the replacement character `�`: ranging
over `"a\xffb"` gives `a`, U+FFFD, and `b`.

### Bytes into text

Text arrives as bytes, from a file or a socket, and has to be decoded. The languages
differ in what they do with bytes that are not valid UTF-8:

| Language | Decode | Invalid UTF-8 |
| --- | --- | --- |
| C, C++ | Nothing to do: the bytes are the string | Nothing checks |
| Rust | `String::from_utf8(bytes)` | Returns `Err`; `String::from_utf8_lossy` puts in U+FFFD instead |
| Go | `string(bytes)` | Kept as they are; `utf8.ValidString(s)` checks |
| Python | `data.decode("utf-8")` | `UnicodeDecodeError: 'utf-8' codec can't decode byte 0xff in position 0: invalid start byte`; `errors="replace"` puts in U+FFFD |
| JavaScript, TypeScript | `new TextDecoder().decode(data)` | Puts in U+FFFD; `new TextDecoder("utf-8", { fatal: true })` throws a `TypeError` |
| Dart | `utf8.decode(data)` | `FormatException: Invalid UTF-8 byte`; `allowMalformed: true` puts in U+FFFD |
| Kotlin | `data.decodeToString()` | Puts in U+FFFD; `decodeToString(throwOnInvalidSequence = true)` throws `MalformedInputException` |

The UTF-16 languages have the opposite problem. Their strings can hold half of a
surrogate pair, which is not valid text, and encoding it as UTF-8 quietly writes U+FFFD
(`EF BF BD`) instead. JavaScript's `s.isWellFormed()` (ES2024) checks for that.

---

## 6.5 Building strings

The program below joins a list, builds a string in a loop, pads two columns, and
formats two numbers. The code is in [`examples/03-building/`](examples/03-building/).

**C**

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *fruits[] = {"apple", "banana", "cherry"};
    const size_t count = sizeof fruits / sizeof fruits[0];

    // C has no growable string. Build into a fixed buffer, and keep track of the end.
    char joined[64] = "";
    for (size_t i = 0; i < count; i++) {
        if (i > 0) {
            strncat(joined, ", ", sizeof joined - strlen(joined) - 1);
        }
        strncat(joined, fruits[i], sizeof joined - strlen(joined) - 1);
    }
    printf("joined: %s\n", joined);

    // snprintf returns how many characters it wrote, so `used` marks the end.
    char built[64];
    int used = 0;
    for (int n = 1; n <= 5; n++) {
        used += snprintf(built + used, sizeof built - (size_t)used, n > 1 ? "-%d" : "%d", n);
    }
    printf("built: %s\n", built);

    printf("padded: [%-8s] [%8d]\n", "Ada", 90);  // - means left-align
    printf("zero-padded: %05d\n", 42);
    printf("two decimals: %.2f\n", 3.14159);
    return 0;
}
```

**C++**

```cpp
#include <format>
#include <iostream>
#include <string>
#include <vector>

int main() {
    const std::vector<std::string> fruits = {"apple", "banana", "cherry"};

    // std::string is mutable and grows as needed, so += in a loop is cheap.
    std::string joined;
    for (std::size_t i = 0; i < fruits.size(); ++i) {
        if (i > 0) {
            joined += ", ";
        }
        joined += fruits[i];
    }
    std::cout << "joined: " << joined << '\n';

    std::string built;
    for (int n = 1; n <= 5; ++n) {
        built += (n > 1 ? "-" : "") + std::to_string(n);
    }
    std::cout << "built: " << built << '\n';

    // std::format (C++20) uses Python-style format specs.
    std::cout << std::format("padded: [{:<8}] [{:>8}]\n", "Ada", 90);
    std::cout << std::format("zero-padded: {:05}\n", 42);
    std::cout << std::format("two decimals: {:.2f}\n", 3.14159);
}
```

**Rust**

```rust
use std::fmt::Write; // lets write! append to a String

fn main() {
    let fruits = ["apple", "banana", "cherry"];
    println!("joined: {}", fruits.join(", "));

    // A String is mutable and grows as needed.
    let mut built = String::new();
    for n in 1..=5 {
        if n > 1 {
            built.push('-');
        }
        write!(built, "{n}").unwrap(); // writing to a String cannot fail
    }
    println!("built: {built}");

    println!("padded: [{:<8}] [{:>8}]", "Ada", 90);
    println!("zero-padded: {:05}", 42);
    println!("two decimals: {:.2}", 3.14159);
}
```

**Go**

```go
package main

import (
	"fmt"
	"strconv"
	"strings"
)

func main() {
	fruits := []string{"apple", "banana", "cherry"}
	fmt.Println("joined:", strings.Join(fruits, ", "))

	// Strings are immutable, so += copies. strings.Builder appends in place.
	var built strings.Builder
	for n := 1; n <= 5; n++ {
		if n > 1 {
			built.WriteByte('-')
		}
		built.WriteString(strconv.Itoa(n))
	}
	fmt.Println("built:", built.String())

	fmt.Printf("padded: [%-8s] [%8d]\n", "Ada", 90) // - means left-align
	fmt.Printf("zero-padded: %05d\n", 42)
	fmt.Printf("two decimals: %.2f\n", 3.14159)
}
```

**Python**

```python
def main() -> None:
    fruits = ["apple", "banana", "cherry"]
    print("joined:", ", ".join(fruits))

    # Strings are immutable, so build the pieces first and join them once.
    print("built:", "-".join(str(n) for n in range(1, 6)))

    print(f"padded: [{'Ada':<8}] [{90:>8}]")
    print(f"zero-padded: {42:05}")
    print(f"two decimals: {3.14159:.2f}")


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
const fruits = ["apple", "banana", "cherry"];
console.log("joined:", fruits.join(", "));

// Strings are immutable, but engines make += in a loop fast, so this is fine.
let built = "";
for (let n = 1; n <= 5; n++) {
  built += n > 1 ? `-${n}` : `${n}`;
}
console.log("built:", built);

console.log(`padded: [${"Ada".padEnd(8)}] [${String(90).padStart(8)}]`);
console.log(`zero-padded: ${String(42).padStart(5, "0")}`);
console.log(`two decimals: ${(3.14159).toFixed(2)}`);
```

**TypeScript**

```ts
const fruits: string[] = ["apple", "banana", "cherry"];
console.log("joined:", fruits.join(", "));

// Strings are immutable, but engines make += in a loop fast, so this is fine.
let built = "";
for (let n = 1; n <= 5; n++) {
  built += n > 1 ? `-${n}` : `${n}`;
}
console.log("built:", built);

console.log(`padded: [${"Ada".padEnd(8)}] [${String(90).padStart(8)}]`);
console.log(`zero-padded: ${String(42).padStart(5, "0")}`);
console.log(`two decimals: ${(3.14159).toFixed(2)}`);

export {};
```

**Dart**

```dart
void main() {
  const fruits = ['apple', 'banana', 'cherry'];
  print('joined: ${fruits.join(', ')}');

  // Strings are immutable. StringBuffer collects pieces and builds the string once.
  final built = StringBuffer();
  for (var n = 1; n <= 5; n++) {
    if (n > 1) built.write('-');
    built.write(n);
  }
  print('built: $built');

  print('padded: [${'Ada'.padRight(8)}] [${'90'.padLeft(8)}]');
  print('zero-padded: ${'42'.padLeft(5, '0')}');
  print('two decimals: ${3.14159.toStringAsFixed(2)}');
}
```

**Kotlin**

```kotlin
import java.util.Locale

fun main() {
    val fruits = listOf("apple", "banana", "cherry")
    println("joined: ${fruits.joinToString(", ")}")

    // Strings are immutable. buildString gives the block a StringBuilder to append to.
    val built = buildString {
        for (n in 1..5) {
            if (n > 1) append('-')
            append(n)
        }
    }
    println("built: $built")

    println("padded: [${"Ada".padEnd(8)}] [${"90".padStart(8)}]")
    println("zero-padded: ${"42".padStart(5, '0')}")
    println("two decimals: ${String.format(Locale.ROOT, "%.2f", 3.14159)}")
}
```

Output, identical in all nine:

```text
joined: apple, banana, cherry
built: 1-2-3-4-5
padded: [Ada     ] [      90]
zero-padded: 00042
two decimals: 3.14
```

| Language | Join a list | Build in a loop | Repeat |
| --- | --- | --- | --- |
| C | A loop with `strncat` or `snprintf` into a buffer you size | A `char` array and the position of its end | A loop |
| C++ | A loop with `+=` (C++23 adds `std::views::join_with`) | `std::string` itself, with `+=`; `reserve` first if you know the size | `std::string(3, '-')`, for one character |
| Rust | `parts.join(", ")` | `String` itself: `push`, `push_str`, and `write!` | `"ab".repeat(3)` |
| Go | `strings.Join(parts, ", ")` | `strings.Builder` | `strings.Repeat("ab", 3)` |
| Python | `", ".join(parts)`; every item must be a `str` | A list of parts, then `join` | `"ab" * 3` |
| JavaScript, TypeScript | `parts.join(", ")` | `+=` | `"ab".repeat(3)` |
| Dart | `parts.join(', ')` | `StringBuffer` | `'ab' * 3` |
| Kotlin | `parts.joinToString(", ")` | `buildString { ... }`, or a `StringBuilder` | `"ab".repeat(3)` |

**Why a builder.** When strings cannot change, `s += x` makes a new string and copies
both parts into it. In a loop, the copies add up: appending n pieces one at a time
copies about n²/2 characters. On the machine that tested this book, 200,000 appends
of one character took about 4 seconds in Go, 3 in Dart, and 6 in Kotlin. A
`strings.Builder` or `buildString` did the same work in a few milliseconds.

JavaScript engines and CPython avoid most of that copying. V8 links the pieces into a
tree, called a **rope**, and copies them into one flat string only when it needs to.
CPython grows a string in place when a local variable holds the only reference to it.
The same loop took about 15 ms in Node and 10 ms inside a Python function, but neither
language promises this, and Python's documentation recommends `join`.

C's `strcat` is slow in a loop for another reason: it walks from the start of the
string to find the end on every call. The C example uses `strncat` for three words,
where that does not matter, and keeps the end in `used` for the loop.

C++ and Rust strings can grow in place, like a growable array. `+=` and `push_str`
add to the end, and the buffer grows to about twice its size when it runs out of room,
so a loop of appends is fast without a separate builder.

### Interpolation and format strings

| Language | Interpolation | Format call |
| --- | --- | --- |
| C | None | `printf("%s is %d", name, age)`, `snprintf` into a buffer |
| C++ | None | `std::format("{} is {}", name, age)` (C++20) |
| Rust | `format!("{name} is {age}")`, for plain variable names only | `format!("{} is {}", name, age)` |
| Go | None | `fmt.Sprintf("%s is %d", name, age)` |
| Python | `f"{name} is {age}"`, any expression | `"{} is {}".format(name, age)` |
| JavaScript, TypeScript | `` `${name} is ${age}` ``, any expression | None built in |
| Dart | `'$name is $age'`, and `'${user.name}'` for an expression | None built in |
| Kotlin | `"$name is $age"`, and `"${user.name}"` for an expression | `"%s is %d".format(name, age)` |

Rust's inline names cannot reach into a value: `format!("{user.name}")` is an error,
`invalid format string: field access isn't supported`. Write
`format!("{}", user.name)`.

Python's `f"{x=}"` prints the expression and its value, `x=5`, which helps when
debugging. Rust's `{x:?}` prints a value's debug form, with quotes around strings.

### Format specifications

The languages share two families of format specification:

| Want | C, Go, Kotlin (`printf` style) | C++, Rust, Python (`{}` style) | JavaScript, TypeScript | Dart |
| --- | --- | --- | --- | --- |
| Left-align in 8 | `%-8s` | `{:<8}` | `s.padEnd(8)` | `s.padRight(8)` |
| Right-align in 8 | `%8d` | `{:>8}` | `s.padStart(8)` | `s.padLeft(8)` |
| Zero-pad to 5 | `%05d` | `{:05}` | `String(n).padStart(5, "0")` | `'$n'.padLeft(5, '0')` |
| Two decimals | `%.2f` | `{:.2f}` in C++ and Python, `{:.2}` in Rust | `x.toFixed(2)` | `x.toStringAsFixed(2)` |

Kotlin also has `padEnd` and `padStart`, which the example uses.

Watch the precision. In C++ and Python, `{:.2}` means two significant digits, so
3.14159 prints as `3.1`. In Rust, `{:.2}` means two decimals, `3.14`, and `{:.2f}` is a
compile error: `unknown format trait 'f'`.

**Locale.** Kotlin's `"%.2f".format(x)` and `String.format` use the computer's locale,
so on a German system 3.14159 prints as `3,14`. The example passes `Locale.ROOT` to get
a dot every time. C's `printf` uses a dot unless the program calls `setlocale`. The other
languages always use a dot unless you ask for a locale, as with JavaScript's
`toLocaleString("de")`.

### Padding text that is not ASCII

Padding to a width means counting the text's length, and each language counts in its
own unit. Here is `"Zoë"` padded on the right and `"👍"` padded on the left, both to a
width of 6:

| Language | Counts | `"Zoë"` gets | `"👍"` gets |
| --- | --- | --- | --- |
| C (`%-6s`) | Bytes | 2 spaces | 2 spaces |
| C++ (`{:<6}`), with GCC 13 | Bytes | 2 spaces | 2 spaces |
| Rust, Go, Python | Code points | 3 spaces | 5 spaces |
| JavaScript, TypeScript, Dart, Kotlin | UTF-16 code units | 3 spaces | 4 spaces |

None of them measures how wide the text looks. Most terminals draw 👍 two columns
wide, so even counting code points leaves the column one space off. The C++ standard
asks `std::format` to estimate that display width, and GCC's library does so from
version 14. For tables of text that is not ASCII, measure the width yourself, or use a
library for it.

---

## 6.6 Everyday operations

The program below trims a string, splits it into words, changes case, searches,
replaces, and sorts three words two ways. The code is in
[`examples/04-operations/`](examples/04-operations/).

**C**

```c
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *yes_no(int condition) { return condition ? "yes" : "no"; }

// strcmp compares bytes, so every uppercase ASCII letter sorts before every lowercase one.
static int compare_bytes(const void *a, const void *b) {
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

static int compare_ignoring_case(const void *a, const void *b) {
    const unsigned char *x = *(const unsigned char *const *)a;
    const unsigned char *y = *(const unsigned char *const *)b;
    while (*x != '\0' && tolower(*x) == tolower(*y)) {
        x++;
        y++;
    }
    return tolower(*x) - tolower(*y);
}

static void print_list(const char *label, const char **items, size_t count) {
    printf("%s:", label);
    for (size_t i = 0; i < count; i++) {
        printf(" %s", items[i]);
    }
    printf("\n");
}

int main(void) {
    const char *text = "  The quick  brown fox  ";

    // Trim: skip spaces at the front, then find the last non-space.
    const char *start = text;
    while (isspace((unsigned char)*start)) {
        start++;
    }
    size_t length = strlen(start);
    while (length > 0 && isspace((unsigned char)start[length - 1])) {
        length--;
    }
    char trimmed[64];
    snprintf(trimmed, sizeof trimmed, "%.*s", (int)length, start);
    printf("trimmed: [%s]\n", trimmed);

    // Split on runs of spaces by walking the string.
    printf("words: ");
    int first = 1;
    for (const char *p = trimmed; *p != '\0';) {
        while (isspace((unsigned char)*p)) {
            p++;
        }
        const char *word = p;
        while (*p != '\0' && !isspace((unsigned char)*p)) {
            p++;
        }
        printf("%s%.*s", first ? "" : "|", (int)(p - word), word);
        first = 0;
    }
    printf("\n");

    char upper[64];
    for (size_t i = 0; i <= length; i++) {  // <= copies the '\0' too
        upper[i] = (char)toupper((unsigned char)trimmed[i]);
    }
    printf("upper: %s\n", upper);

    const char *found = strstr(trimmed, "brown");  // NULL if not found
    printf("contains \"quick\": %s\n", yes_no(strstr(trimmed, "quick") != NULL));
    printf("index of \"brown\": %td\n", found - trimmed);

    // Replace: copy the part before the match, the new text, then the rest.
    const char *match = strstr(trimmed, "quick");
    char replaced[64];
    snprintf(replaced, sizeof replaced, "%.*s%s%s", (int)(match - trimmed), trimmed, "slow",
             match + strlen("quick"));
    printf("replaced: %s\n", replaced);

    printf("starts with \"The\": %s\n", yes_no(strncmp(trimmed, "The", 3) == 0));

    const char *words[] = {"banana", "apple", "Cherry"};
    qsort(words, 3, sizeof words[0], compare_bytes);
    print_list("sorted", words, 3);
    qsort(words, 3, sizeof words[0], compare_ignoring_case);
    print_list("sorted ignoring case", words, 3);
    return 0;
}
```

**C++**

```cpp
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

const char* yes_no(bool condition) { return condition ? "yes" : "no"; }

// std::tolower takes an int that must fit in unsigned char, hence the cast.
char lower(char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
char upper(char c) { return static_cast<char>(std::toupper(static_cast<unsigned char>(c))); }

void print_list(const std::string& label, const std::vector<std::string>& items) {
    std::cout << label << ':';
    for (const auto& item : items) {
        std::cout << ' ' << item;
    }
    std::cout << '\n';
}

int main() {
    const std::string text = "  The quick  brown fox  ";

    const auto first = text.find_first_not_of(" \t\n");
    const auto last = text.find_last_not_of(" \t\n");
    const std::string trimmed = text.substr(first, last - first + 1);
    std::cout << "trimmed: [" << trimmed << "]\n";

    // >> reads one word at a time, skipping any amount of whitespace.
    std::istringstream stream(trimmed);
    std::string word;
    std::string words;
    while (stream >> word) {
        words += (words.empty() ? "" : "|") + word;
    }
    std::cout << "words: " << words << '\n';

    std::string shouted = trimmed;
    std::transform(shouted.begin(), shouted.end(), shouted.begin(), upper);
    std::cout << "upper: " << shouted << '\n';

    // find returns std::string::npos when the text is not there.
    std::cout << "contains \"quick\": " << yes_no(trimmed.find("quick") != std::string::npos) << '\n';
    std::cout << "index of \"brown\": " << trimmed.find("brown") << '\n';

    std::string replaced = trimmed;
    replaced.replace(replaced.find("quick"), std::string("quick").size(), "slow");
    std::cout << "replaced: " << replaced << '\n';

    std::cout << "starts with \"The\": " << yes_no(trimmed.starts_with("The")) << '\n';

    std::vector<std::string> fruits = {"banana", "apple", "Cherry"};
    std::sort(fruits.begin(), fruits.end());  // compares bytes
    print_list("sorted", fruits);
    std::sort(fruits.begin(), fruits.end(), [](const std::string& a, const std::string& b) {
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
                                            [](char x, char y) { return lower(x) < lower(y); });
    });
    print_list("sorted ignoring case", fruits);
}
```

**Rust**

```rust
fn yes_no(condition: bool) -> &'static str {
    if condition { "yes" } else { "no" }
}

fn main() {
    let text = "  The quick  brown fox  ";

    let trimmed = text.trim(); // a &str pointing into text: nothing is copied
    println!("trimmed: [{trimmed}]");

    let words: Vec<&str> = trimmed.split_whitespace().collect();
    println!("words: {}", words.join("|"));

    println!("upper: {}", trimmed.to_uppercase());
    println!("contains \"quick\": {}", yes_no(trimmed.contains("quick")));
    // find returns Option<usize>: a byte index, or None.
    println!("index of \"brown\": {}", trimmed.find("brown").unwrap());
    println!("replaced: {}", trimmed.replace("quick", "slow"));
    println!(
        "starts with \"The\": {}",
        yes_no(trimmed.starts_with("The"))
    );

    let mut fruits = vec!["banana", "apple", "Cherry"];
    fruits.sort(); // compares bytes
    println!("sorted: {}", fruits.join(" "));
    fruits.sort_by_key(|s| s.to_lowercase());
    println!("sorted ignoring case: {}", fruits.join(" "));
}
```

**Go**

```go
package main

import (
	"fmt"
	"slices"
	"strings"
)

func yesNo(condition bool) string {
	if condition {
		return "yes"
	}
	return "no"
}

func main() {
	text := "  The quick  brown fox  "

	trimmed := strings.TrimSpace(text)
	fmt.Printf("trimmed: [%s]\n", trimmed)

	// Fields splits on runs of whitespace; Split(s, " ") would give empty strings.
	fmt.Println("words:", strings.Join(strings.Fields(trimmed), "|"))

	fmt.Println("upper:", strings.ToUpper(trimmed))
	fmt.Println(`contains "quick":`, yesNo(strings.Contains(trimmed, "quick")))
	fmt.Println(`index of "brown":`, strings.Index(trimmed, "brown")) // -1 if missing
	fmt.Println("replaced:", strings.ReplaceAll(trimmed, "quick", "slow"))
	fmt.Println(`starts with "The":`, yesNo(strings.HasPrefix(trimmed, "The")))

	fruits := []string{"banana", "apple", "Cherry"}
	slices.Sort(fruits) // compares bytes
	fmt.Println("sorted:", strings.Join(fruits, " "))
	slices.SortFunc(fruits, func(a, b string) int {
		return strings.Compare(strings.ToLower(a), strings.ToLower(b))
	})
	fmt.Println("sorted ignoring case:", strings.Join(fruits, " "))
}
```

**Python**

```python
def yes_no(condition: bool) -> str:
    return "yes" if condition else "no"


def main() -> None:
    text = "  The quick  brown fox  "

    trimmed = text.strip()
    print(f"trimmed: [{trimmed}]")
    # split() with no argument splits on runs of whitespace.
    print("words:", "|".join(trimmed.split()))
    print("upper:", trimmed.upper())
    print('contains "quick":', yes_no("quick" in trimmed))
    print('index of "brown":', trimmed.find("brown"))  # -1 if missing; index() raises
    print("replaced:", trimmed.replace("quick", "slow"))
    print('starts with "The":', yes_no(trimmed.startswith("The")))

    fruits = ["banana", "apple", "Cherry"]
    print("sorted:", " ".join(sorted(fruits)))  # compares code points
    print("sorted ignoring case:", " ".join(sorted(fruits, key=str.lower)))


if __name__ == "__main__":
    main()
```

**JavaScript**

```js
const yesNo = (condition) => (condition ? "yes" : "no");

const text = "  The quick  brown fox  ";

const trimmed = text.trim();
console.log(`trimmed: [${trimmed}]`);
// split(" ") would give an empty string between the two spaces; a regex splits on runs.
console.log("words:", trimmed.split(/\s+/).join("|"));
console.log("upper:", trimmed.toUpperCase());
console.log('contains "quick":', yesNo(trimmed.includes("quick")));
console.log('index of "brown":', trimmed.indexOf("brown")); // -1 if missing
console.log("replaced:", trimmed.replaceAll("quick", "slow"));
console.log('starts with "The":', yesNo(trimmed.startsWith("The")));

const fruits = ["banana", "apple", "Cherry"];
fruits.sort(); // compares UTF-16 code units
console.log("sorted:", fruits.join(" "));
fruits.sort((a, b) => {
  const x = a.toLowerCase();
  const y = b.toLowerCase();
  return x < y ? -1 : x > y ? 1 : 0;
});
console.log("sorted ignoring case:", fruits.join(" "));
```

**TypeScript**

```ts
const yesNo = (condition: boolean): string => (condition ? "yes" : "no");

const text = "  The quick  brown fox  ";

const trimmed = text.trim();
console.log(`trimmed: [${trimmed}]`);
// split(" ") would give an empty string between the two spaces; a regex splits on runs.
console.log("words:", trimmed.split(/\s+/).join("|"));
console.log("upper:", trimmed.toUpperCase());
console.log('contains "quick":', yesNo(trimmed.includes("quick")));
console.log('index of "brown":', trimmed.indexOf("brown")); // -1 if missing
console.log("replaced:", trimmed.replaceAll("quick", "slow"));
console.log('starts with "The":', yesNo(trimmed.startsWith("The")));

const fruits: string[] = ["banana", "apple", "Cherry"];
fruits.sort(); // compares UTF-16 code units
console.log("sorted:", fruits.join(" "));
fruits.sort((a, b) => {
  const x = a.toLowerCase();
  const y = b.toLowerCase();
  return x < y ? -1 : x > y ? 1 : 0;
});
console.log("sorted ignoring case:", fruits.join(" "));

export {};
```

**Dart**

```dart
String yesNo(bool condition) => condition ? 'yes' : 'no';

void main() {
  const text = '  The quick  brown fox  ';

  final trimmed = text.trim();
  print('trimmed: [$trimmed]');
  print('words: ${trimmed.split(RegExp(r'\s+')).join('|')}');
  print('upper: ${trimmed.toUpperCase()}');
  print('contains "quick": ${yesNo(trimmed.contains('quick'))}');
  print('index of "brown": ${trimmed.indexOf('brown')}'); // -1 if missing
  print('replaced: ${trimmed.replaceAll('quick', 'slow')}');
  print('starts with "The": ${yesNo(trimmed.startsWith('The'))}');

  final fruits = ['banana', 'apple', 'Cherry'];
  fruits.sort(); // compares UTF-16 code units
  print('sorted: ${fruits.join(' ')}');
  fruits.sort((a, b) => a.toLowerCase().compareTo(b.toLowerCase()));
  print('sorted ignoring case: ${fruits.join(' ')}');
}
```

**Kotlin**

```kotlin
fun yesNo(condition: Boolean) = if (condition) "yes" else "no"

fun main() {
    val text = "  The quick  brown fox  "

    val trimmed = text.trim()
    println("trimmed: [$trimmed]")
    println("words: ${trimmed.split(Regex("\\s+")).joinToString("|")}")
    println("upper: ${trimmed.uppercase()}")
    println("contains \"quick\": ${yesNo("quick" in trimmed)}")
    println("index of \"brown\": ${trimmed.indexOf("brown")}") // -1 if missing
    println("replaced: ${trimmed.replace("quick", "slow")}")
    println("starts with \"The\": ${yesNo(trimmed.startsWith("The"))}")

    val fruits = mutableListOf("banana", "apple", "Cherry")
    fruits.sort() // compares UTF-16 code units
    println("sorted: ${fruits.joinToString(" ")}")
    fruits.sortWith(String.CASE_INSENSITIVE_ORDER)
    println("sorted ignoring case: ${fruits.joinToString(" ")}")
}
```

Output, identical in all nine:

```text
trimmed: [The quick  brown fox]
words: The|quick|brown|fox
upper: THE QUICK  BROWN FOX
contains "quick": yes
index of "brown": 11
replaced: The slow  brown fox
starts with "The": yes
sorted: Cherry apple banana
sorted ignoring case: apple banana Cherry
```

| Language | Trim | Split on runs of spaces | Split on a separator | Uppercase |
| --- | --- | --- | --- | --- |
| C | By hand, with `isspace` | By hand, or `strtok`, which writes `'\0'` into the string | `strtok` | `toupper` on each byte |
| C++ | `find_first_not_of` and `find_last_not_of` | `std::istringstream` and `>>` | `std::views::split` (C++20) | `std::transform` with `toupper` |
| Rust | `s.trim()` | `s.split_whitespace()` | `s.split(',')` | `s.to_uppercase()` |
| Go | `strings.TrimSpace(s)` | `strings.Fields(s)` | `strings.Split(s, ",")` | `strings.ToUpper(s)` |
| Python | `s.strip()` | `s.split()` | `s.split(",")` | `s.upper()` |
| JavaScript, TypeScript | `s.trim()` | `s.trim().split(/\s+/)` | `s.split(",")` | `s.toUpperCase()` |
| Dart | `s.trim()` | `s.trim().split(RegExp(r'\s+'))` | `s.split(',')` | `s.toUpperCase()` |
| Kotlin | `s.trim()` | `s.trim().split(Regex("\\s+"))` | `s.split(",")` | `s.uppercase()` |

| Language | Contains | Position of | Not found | Replace every match | Starts with |
| --- | --- | --- | --- | --- | --- |
| C | `strstr(s, x) != NULL` | `strstr(s, x) - s` | `NULL` | By hand, as in the example | `strncmp(s, x, strlen(x)) == 0` |
| C++ | `s.find(x) != std::string::npos`; `s.contains(x)` in C++23 | `s.find(x)` | `std::string::npos` | A loop of `find` and `replace`, or `std::regex_replace` | `s.starts_with(x)` (C++20) |
| Rust | `s.contains(x)` | `s.find(x)` | `None` | `s.replace(x, y)` | `s.starts_with(x)` |
| Go | `strings.Contains(s, x)` | `strings.Index(s, x)` | `-1` | `strings.ReplaceAll(s, x, y)` | `strings.HasPrefix(s, x)` |
| Python | `x in s` | `s.find(x)` or `s.index(x)` | `-1`; `index` raises `ValueError: substring not found` | `s.replace(x, y)` | `s.startswith(x)` |
| JavaScript, TypeScript | `s.includes(x)` | `s.indexOf(x)` | `-1` | `s.replaceAll(x, y)` | `s.startsWith(x)` |
| Dart | `s.contains(x)` | `s.indexOf(x)` | `-1` | `s.replaceAll(x, y)` | `s.startsWith(x)` |
| Kotlin | `x in s` | `s.indexOf(x)` | `-1` | `s.replace(x, y)` | `s.startsWith(x)` |

Positions come back in each language's own unit: bytes in C, C++, Rust, and Go, code
points in Python, and UTF-16 units in the rest. The example's text is ASCII, so all
nine print 11.

**Replacing once or everywhere.** JavaScript's `replace` with a string replaces only
the first match: `"aa".replace("a", "b")` is `"ba"`. Use `replaceAll` (ES2021), or a
regular expression with the `g` flag. In Rust, Python, and Kotlin, `replace` replaces
every match, and `replacen(x, y, 1)`, `replace(x, y, 1)`, and `replaceFirst` replace
one. Dart names both: `replaceFirst` and `replaceAll`. Go's `strings.Replace` takes a
count, and `-1` means all.

### Splitting surprises

| Expression | Result |
| --- | --- |
| `"a,b,,".split(",")` | Four items, the last two empty, in Rust, Go, Python, JavaScript, TypeScript, Dart, and Kotlin. Java's `split` drops trailing empty strings and returns two, which matters when you port Java code to Kotlin |
| `"".split(",")` | One empty string, not an empty list, in all seven |
| `"a  b".split(" ")` | Three items, with an empty string between the two spaces |
| Splitting `""` on whitespace | Python's `split()`, Go's `Fields`, and Rust's `split_whitespace` return nothing. JavaScript's `"".trim().split(/\s+/)` returns `[""]`, and so do the Dart and Kotlin forms above, so counting words that way says an empty line has one word |

### Case is not one character to one

Changing case is harder than it looks. The German `ß` has no single uppercase letter in
everyday use, and becomes `SS`. Turkish has a dotted and a dotless i, so uppercase `i`
is `İ` in Turkish and `I` everywhere else.

| Language | `"straße"` in uppercase | Turkish rules |
| --- | --- | --- |
| C, C++ | `STRAßE`: `toupper` changes only the ASCII letters in the default locale, one byte at a time | None |
| Rust | `STRASSE` | None in the standard library; `to_uppercase` never depends on the locale |
| Go | `STRAßE`: `ToUpper` maps each code point to exactly one code point | `strings.ToUpperSpecial(unicode.TurkishCase, s)` |
| Python | `STRASSE` | None; `upper` never depends on the locale |
| JavaScript, TypeScript | `STRASSE` | `s.toLocaleUpperCase("tr")` |
| Dart | `STRAßE` on the Dart VM, but `STRASSE` when compiled to JavaScript | None in the core libraries |
| Kotlin | `STRASSE` | `s.uppercase(Locale.forLanguageTag("tr"))` |

Kotlin's choice avoids a well-known Java trap. Java's `toUpperCase()` uses the computer's
locale, so on a computer set to Turkish, `"title".toUpperCase()` is `"TİTLE"`, and any
comparison with `"TITLE"` fails. Kotlin's `uppercase()` and `lowercase()` always use
the root locale, and Kotlin 2.4 rejects the old Java-style name with an error:
`'fun String.toUpperCase(): String' is deprecated. Use uppercase() instead.`

**Comparing without case.** Lowercasing both sides works for ASCII. For other text,
`"Straße"` and `"STRASSE"` are the same German word, and only some tools agree:

| Language | Ignoring case | `"Straße"` and `"STRASSE"` |
| --- | --- | --- |
| C | `strcasecmp(a, b)`, from POSIX `<strings.h>`; ASCII only | Different |
| C++ | None built in; compare `tolower` of each byte | Different |
| Rust | `a.eq_ignore_ascii_case(b)`, or compare `to_lowercase()` of both | Different |
| Go | `strings.EqualFold(a, b)` | Different |
| Python | `a.casefold() == b.casefold()` | **Equal**: `casefold` turns `ß` into `ss` |
| JavaScript, TypeScript | `a.localeCompare(b, "de", { sensitivity: "base" }) === 0` | **Equal** |
| Dart | Compare `toLowerCase()` of both | Different |
| Kotlin | `a.equals(b, ignoreCase = true)` | Different |

---

## 6.7 Comparing and sorting

Chapter 3 showed that `==` compares the characters of two strings in every language
except C, where it compares two addresses and you call `strcmp` instead. Ordering is
where the languages differ. Every default comparison works on numbers, not on what a
reader would call alphabetical order, so every uppercase ASCII letter comes before every
lowercase one, and accented letters come after `z`. Example 4 shows the first problem:
`Cherry` sorts before `apple`.

| Language | Default order compares | `["zebra", "élan", "apple", "Zoë"]` sorted | Sorting for people |
| --- | --- | --- | --- |
| C | Bytes, with `strcmp` | `Zoë apple zebra élan` | `strcoll`, after `setlocale` and only with the system's locales |
| C++ | Bytes | `Zoë apple zebra élan` | `std::locale`'s collate facet, which also depends on the system |
| Rust | Bytes, which gives code point order | `Zoë apple zebra élan` | The `icu_collator` crate |
| Go | Bytes | `Zoë apple zebra élan` | `golang.org/x/text/collate` |
| Python | Code points | `Zoë apple zebra élan` | `locale.strxfrm` with the system's locales, or the third-party PyICU |
| JavaScript, TypeScript | UTF-16 code units | `Zoë apple zebra élan` | `a.localeCompare(b)` or `Intl.Collator`: `apple élan zebra Zoë` |
| Dart | UTF-16 code units | `Zoë apple zebra élan` | None in the core libraries |
| Kotlin | UTF-16 code units | `Zoë apple zebra élan` | `java.text.Collator`: `apple élan zebra Zoë` |

**UTF-16 order is not code point order.** For text below U+FFFF the two agree. Above
it they do not, because a surrogate unit (D800 to DFFF) is smaller than the code points
from E000 to FFFF. Sort `["Ａ", "😀"]` (U+FF21, a fullwidth A, and U+1F600). Python,
Go, and Rust put `Ａ` first, because FF21 is less than 1F600. JavaScript, Dart, and
Kotlin put `😀` first, because its first unit, D83D, is less than FF21. This matters
when two systems must agree on an order, such as a database and a JavaScript client
paging through the same sorted list.

**Normalize before comparing.** `"café"` written with U+00E9 and `"café"` written with
`e` and U+0301 look the same and are unequal in all nine languages, because their code
points differ. Normalize both to NFC before you compare or store them:

| Language | Normalize to NFC |
| --- | --- |
| C, C++ | Not in the standard library; ICU is the usual choice |
| Rust | The `unicode-normalization` crate |
| Go | `golang.org/x/text/unicode/norm`: `norm.NFC.String(s)` |
| Python | `unicodedata.normalize("NFC", s)` |
| JavaScript, TypeScript | `s.normalize("NFC")` |
| Dart | Not in the core libraries; use a package |
| Kotlin | `java.text.Normalizer.normalize(s, Normalizer.Form.NFC)` |

---

## 6.8 Writing string literals

| Language | A code point by number | Raw string | Over several lines | `"a" "b"` joins |
| --- | --- | --- | --- | --- |
| C | `"\u00eb"`, `"\U0001F44D"` | None | `"line 1\n" "line 2\n"` | Yes |
| C++ | The same | `R"(C:\new)"`, or `R"x(...)x"` when the text contains `)"` | Raw strings can span lines | Yes |
| Rust | `"\u{eb}"`, `"\u{1F44D}"` | `r"C:\new"`, and `r#"say "hi""#` for quotes | Ordinary strings can span lines. A `\` at the end of a line skips the newline and the next line's leading spaces | No; the `concat!` macro joins literals |
| Go | `"\u00eb"`, `"\U0001F44D"` | `` `C:\new` ``, which cannot contain a backtick | Backtick strings | No |
| Python | `"\u00eb"`, `"\U0001F44D"`, `"\N{THUMBS UP SIGN}"` | `r"C:\new"` | `"""..."""` | Yes |
| JavaScript, TypeScript | `"\u00eb"`, `"\u{1F44D}"` | ``String.raw`C:\new` `` | Template literals, `` `...` `` | No |
| Dart | `'\u00eb'`, `'\u{1F44D}'` | `r'C:\new'` | `'''...'''` or `"""..."""` | Yes |
| Kotlin | `"\u00eb"`; exactly four hex digits, so 👍 is `"\uD83D\uDC4D"` | `"""C:\new"""`, which still interpolates `$` | `"""...""".trimIndent()` | No |

C and C++ store a character written as `\u` or `\U` in the compiler's execution
character set, which is UTF-8 by default in GCC and Clang. C++20 added `u8"..."`
literals for UTF-8, but gave them a new type, `const char8_t[]`, so `std::string s =
u8"Zoë";` no longer compiles. GCC reports `conversion from 'const char8_t [5]' to
non-scalar type 'std::string' ... requested`.

**Dollar signs in Kotlin.** Because `$` starts a template in every Kotlin string, a
literal dollar sign before a name needs care. In an ordinary string, write `\$price`.
A raw string has no escapes, so you had to write `${'$'}price`. Kotlin 2.2 made
multi-dollar strings a stable feature: with `$$` before the opening quotes, it takes
`$$` to start a template, and a single `$` is just a character.

```kotlin
val price = 5
println($$"""{"name": "$price", "value": $$price}""") // {"name": "$price", "value": 5}
```

Dart has the same issue with `$`, and solves it with `\$` or a raw string, `r'$price'`,
which turns off interpolation entirely.

---

## 6.9 Why each language looks the way it does

- **C** (1972) treats a string as an array of `char` that ends with a zero byte. Dennis
  Ritchie chose the terminator over a stored length partly to avoid a length limit and
  partly because it was more convenient. C has no idea what encoding the bytes use,
  which is why the examples count code points by hand, and why `toupper` works one byte
  at a time.
- **C++** added `std::string` in 1998: bytes again, but with a stored length, value
  semantics, and memory that grows by itself. Unicode was left to libraries. C++20's
  `char8_t` split UTF-8 text from plain `char`, which is why `u8"..."` no longer fits a
  `std::string`.
- **Rust** (1.0 in 2015) arrived after UTF-8 had won. A `String` is always valid UTF-8,
  checked when it is made, so every later operation can trust it. Integer indexing is
  left out because it cannot be both fast and correct for UTF-8.
- **Go**'s designers include Rob Pike and Ken Thompson, who designed UTF-8 in 1992.
  Go strings are bytes, source files are UTF-8, and `range` decodes UTF-8 for you, but
  nothing forces a string to be valid.
- **Python** 3 (2008) split text, `str`, from bytes, `bytes`, which Python 2 had mixed
  together. A `str` is a sequence of code points, and CPython's flexible storage (PEP
  393, Python 3.3) keeps indexing instant while using one byte per character for ASCII
  text.
- **JavaScript**'s first standard (1997) defined a string as a sequence of 16-bit
  values, following early Unicode, which expected 65,536 characters to be enough.
  Unicode 2.0 (1996) had already gone past that with surrogate pairs, so in practice
  JavaScript strings are UTF-16 code units. ES2015 added the code point tools:
  `codePointAt`, `for...of` over a string, and `\u{...}` escapes.
- **TypeScript** uses JavaScript's strings unchanged, and its types cannot tell a
  string of code units from a string of characters.
- **Dart** was built to compile to JavaScript, so its strings match JavaScript's:
  UTF-16 code units, with `runes` for code points. The Dart team publishes the
  `characters` package for grapheme clusters.
- **Kotlin** on the JVM uses Java's strings, which were UTF-16 code units from Java
  1.0 (1996), with surrogate pairs supported since Java 5. Kotlin added string
  templates, raw strings, `buildString`, and case conversion that does not change with
  the computer's locale.

---

## 6.10 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | `char *p = "abc"; p[0] = 'x';` | Compiles with no warning; undefined behavior, a segmentation fault on Linux |
| C | `isupper(c)` or `toupper(c)` on a `char` holding a UTF-8 byte | Undefined behavior for negative values; cast to `unsigned char` first |
| C | `strncpy(dst, src, n)` when `src` has `n` or more bytes | `dst` gets no `'\0'` at the end |
| C | `strcat` in a loop | Walks from the start every time; keep track of the end yourself |
| C++ | `std::string s = "count: " + n;` with an `int n` | Pointer arithmetic: with `n` equal to 5, `s` is `": "`. GCC 13 gives no warning; Clang warns `adding 'int' to a string does not append to the string` |
| C++ | `std::string_view v = a + b;` | `v` points into a temporary that is already gone. It compiles silently and may even print the right text; AddressSanitizer (chapter 19) catches it |
| C++ | `std::string s = u8"Zoë";` | Compile error in C++20: `u8` literals are `char8_t` |
| C++, Python | `{:.2}` to get two decimals | Two significant digits: `3.1`. Write `{:.2f}` |
| Rust | `&s[..n]` with `n` from somewhere else | Panics if `n` falls inside a character; use `s.get(..n)` or `char_indices` |
| Rust | `let t = s + "x";` with `s: &str` | E0369: `cannot add '&str' to '&str'`; write `s.to_owned() + "x"` or `format!` |
| Rust | `format!("{user.name}")` | Compile error: `field access isn't supported` |
| Rust | `{:.2f}`, as in Python | Compile error: `unknown format trait 'f'`; write `{:.2}` |
| Go | `string(65)` | `"A"`, not `"65"`. `go vet` says `conversion from int to string yields a string of one rune, not a string of digits`; use `strconv.Itoa` |
| Go | `for i := 0; i < len(s); i++ { s[i] ... }` | Bytes, not characters; use `for i, r := range s` |
| Go | `s += x` in a long loop | Copies the whole string each time; use `strings.Builder` |
| Python | `"age: " + 5` | `TypeError: can only concatenate str (not "int") to str`; use an f-string |
| Python | `b"a" + "b"` | `TypeError: can't concat str to bytes`; decode or encode first |
| Python | `"they're".title()` | `"They'Re"`: `title` starts a word after any non-letter |
| JavaScript | `s.replace("a", "b")` | Replaces only the first match |
| JavaScript, TypeScript | `s.split("")`, or `s.length` as a character count | Splits 👍 into two halves; counts it as 2 |
| JavaScript, TypeScript | `[...s].reverse().join("")` | Reverses code points, so 👍🏽 comes apart into a skin-tone swatch and 👍 |
| Dart | `'straße'.toUpperCase()` | `STRAßE` on the VM and `STRASSE` on the web |
| Dart | `s.length` as a limit on what a user typed | Counts UTF-16 units; use the `characters` package |
| Kotlin | `"%.2f".format(x)` in a file or protocol | Uses the computer's locale: `3,14` in Germany. Pass `Locale.ROOT` |
| Kotlin | `"Price: $price"` meant literally | Interpolates the variable; write `\$price` |
| Kotlin | `s += x` in a long loop | Copies the whole string each time; use `buildString` |
| Kotlin | Porting `split` from Java | Kotlin keeps trailing empty strings; Java drops them |
| All | Comparing text from two sources without normalizing | Two spellings of `café` compare unequal |
| All | Truncating to n bytes, units, or code points | Can cut a character, or a grapheme, in half |

---

## 6.11 Idiom check

**C: carry the length.** Pass a pointer and a length instead of copying words out of a
string, and print them with `%.*s`. Use `snprintf`, never `sprintf` or `strcpy`, and
cast to `unsigned char` before any `<ctype.h>` call.

```c
#include <stdio.h>

// A borrowed piece of a string: where it starts and how many bytes it has.
typedef struct {
    const char *data;
    size_t length;
} StrView;

// Returns the next space-separated word and moves the cursor past it.
static StrView next_word(const char **cursor) {
    const char *p = *cursor;
    while (*p == ' ') {
        p++;
    }
    const char *start = p;
    while (*p != '\0' && *p != ' ') {
        p++;
    }
    *cursor = p;
    return (StrView){start, (size_t)(p - start)};
}

int main(void) {
    const char *cursor = "  the quick  brown fox";
    for (StrView word = next_word(&cursor); word.length > 0; word = next_word(&cursor)) {
        printf("[%.*s] ", (int)word.length, word.data);  // %.*s prints `length` bytes
    }
    printf("\n");  // [the] [quick] [brown] [fox]
    return 0;
}
```

**C++: take `std::string_view`, return `std::string`.** A function that only reads text
takes a `std::string_view`, which accepts a `std::string`, a literal, or part of
either without copying. Never keep a view longer than the string it points into.
Format with `std::format`, and call `reserve` before a long run of `+=`.

**Rust: take `&str`, return `String`.** A `&str` parameter accepts a literal, a
`&String`, or a slice. Return a `String` when you build new text. Walk text with
`chars()` or `char_indices()`, and slice only at positions those give you.

**Go: `range` for characters, `strings.Builder` for building.** `strings.Fields`,
`strings.Cut` (to split once), and `strings.EqualFold` cover most everyday work. For
many edits, convert to `[]byte` or `[]rune`, edit, and convert back once.

**Python: decode early, encode late.** Turn bytes into `str` as soon as they arrive, work
with `str` everywhere inside the program, and encode only on the way out. This pattern
is often called the **Unicode sandwich**. Always pass `encoding="utf-8"` to `open`,
because without it Python uses the system's encoding. Use f-strings, `join`, and
`casefold()` for comparisons that ignore case.

**JavaScript and TypeScript: `for...of` and `Intl`.** Use template literals for
building, `for...of` or `[...s]` for code points, `Intl.Segmenter` for anything users
count, and `Intl.Collator` or `localeCompare` to sort names. Reach for `replaceAll`
when you mean every match.

**Dart: interpolation and `StringBuffer`.** Write `'$name'` and `'${user.name}'`
instead of `+`, use a `StringBuffer` in loops, raw strings (`r'\d+'`) for regular
expressions, and the `characters` package for user-visible text.

**Kotlin: templates, `buildString`, and `trimIndent`.** Use `"$name"` templates,
`buildString { }` in loops, and `"""...""".trimIndent()` for multi-line text. Use
`uppercase()` and `lowercase()`, pass `Locale.ROOT` to `format` for output a machine
will read, and use `java.text.Collator` to sort for people.

---

## 6.12 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Caesar cipher](exercises/01-caesar/) | Reading lines, working on ASCII letters, leaving every other character alone, modular arithmetic with negative numbers |
| 2 | [Unicode-safe palindromes](exercises/02-palindrome/) | Decoding to code points, filtering, comparing a sequence with its reverse |
| 3 | [Word frequency](exercises/03-word-frequency/) | Splitting text into words, counting with a map, sorting by two keys |

```sh
python3 tools/check.py 06                     # your answers, every language
python3 tools/check.py 06 --lang go,python    # some languages
python3 tools/check.py 06 --solutions         # the reference answers
```

**Stretch:** exercise 2 compares code points, so it says `👍🏽x👍🏽` is not a palindrome:
reversed, the skin-tone modifiers come before the thumbs. In JavaScript, TypeScript, or
Kotlin, change your solution to compare grapheme clusters with `Intl.Segmenter` or
`BreakIterator`, and check that the line becomes a palindrome. Then, in exercise 3, sort
words that have the same count with `Intl.Collator` or `Collator` instead of by code
point. Give it `zebra élan apple`, and `élan` moves from last place to second.

---

Previous: [Chapter 5: Functions and closures](../05-functions-and-closures/) ·
Next: Chapter 7: Built-in collections (coming soon)
