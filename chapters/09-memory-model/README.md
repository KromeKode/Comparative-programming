# Chapter 9: Memory model

Chapters 7 and 8 kept running into the same question: when you assign a list or a
struct to a new variable, do you get a copy, or a second name for the same thing? This
chapter answers it from the bottom up. It looks at how a program's memory is laid out,
what lives on the **stack** and what lives on the **heap**, what a **pointer** and a
**reference** are, and what copying really copies. C, C++, and Rust let you see and
choose where every value lives, so they get most of the chapter. Go, Python,
JavaScript, TypeScript, Dart, and Kotlin hide the heap behind references and a garbage
collector, and they hide it in much the same way, so one section covers all six.

**Languages:** C, C++, and Rust in full; Go, Python, JavaScript, TypeScript, Dart, and
Kotlin together in section 9.8 · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| How memory is laid out: addresses, the stack, the heap, and static data | Ownership (chapter 10), debugging (chapter 19), FFI (chapter 22) |
| What pointers and references are, and how C, C++, and Rust differ | Linked lists and trees (chapters 25 and 26) |
| Why a function can change some arguments and not others | Every function you write |
| Aliasing: two names for one place, and the bugs it causes | Concurrency (chapter 16), security (chapter 39) |
| Shallow and deep copies, and how each language spells them | Design problems (chapter 30) |

---

## 9.1 The concept: places and values

A computer's memory is a long row of bytes, and each byte has a number, its
**address**. A variable is a name for a place in that row, and its value is whatever
bytes are stored there. An `int` usually takes 4 bytes, so a variable `x` holding 5 is
four bytes somewhere, and `&x` in C is the address of the first of them.

A running program splits its memory into regions. Three matter here:

- **Static data** holds global variables and the text of string literals. It is set up
  before `main` starts and lasts until the program ends.
- **The stack** holds each function call's local variables. A call pushes a **frame**
  with room for its locals, and returning pops it, so a local disappears when its
  function returns. Pushing and popping is just moving one pointer, which makes the
  stack very fast, but every frame's size must be known when the code is compiled,
  and the whole stack is small: 8 MiB for the main thread on most Linux systems.
- **The heap** holds memory the program asks for while it runs, in any size it wants,
  for as long as it wants. Something has to give that memory back: the program itself
  in C, C++, and Rust, or a **garbage collector** that finds memory nothing refers to
  any more.

```text
 stack (main's frame)                 heap
┌───────────────────────────┐
│ count  = 3                │
│ scores ───────────────────────────► [90, 85, 77]
│ name   ───────────────────────────► "Ada"
└───────────────────────────┘
```

A **pointer** is a value that holds an address, such as `scores` in the picture. A
**reference** is a pointer with rules attached. A C++ reference can never be null and
can never be pointed somewhere else. A Rust reference is checked by the compiler to
make sure what it points at outlives it. A reference in Python or Kotlin is a pointer
you cannot see, print, or do arithmetic on.

With **value semantics**, assignment copies the value, so two variables are
independent. With **reference semantics**, a variable holds a reference to an object,
assignment copies the reference, and both variables now name one object. Two names for
the same place are called **aliases**, and **aliasing** is harmless until one of the
names writes. Finally, a **shallow copy** copies the top level of a value and shares
everything it points to, while a **deep copy** copies all the way down.

---

## 9.2 What each language gives you

| Language | Where locals live | Asking for heap memory | Pointers | References | Who frees heap memory | `b = a` for a struct or object |
| --- | --- | --- | --- | --- | --- | --- |
| C | Stack | `malloc`, `realloc` | `T *`, with arithmetic | None | You, with `free` | Copies every byte |
| C++ | Stack | Containers, `std::make_unique`, `new` | `T *`, with arithmetic | `T &`: never null, never reseated | Destructors (chapter 10) | Copies each member |
| Rust | Stack | `Box`, `Vec`, `String` | `*const T`, `*mut T`, read only in `unsafe` | `&T` and `&mut T`, checked by the compiler | The compiler, when the owner goes away (chapter 10) | Moves; copies only `Copy` types |
| Go | The compiler decides | `&T{}`, `new`, `make`, `append` | `*T`, no arithmetic | None; pointers do the job | Garbage collector | Copies every field |
| Python | Every object is on the heap | Creating any object | None visible | Every variable is one | Reference counting plus a cycle collector | Shares |
| JavaScript | Objects are on the heap | Creating any object | None | Every object is reached through one | Garbage collector | Shares |
| TypeScript | As JavaScript | As JavaScript | None | As JavaScript | Garbage collector | Shares |
| Dart | Objects are on the heap | Creating any object | None | Every variable is one | Garbage collector | Shares |
| Kotlin | `Int` and friends in place; objects on the heap | Creating any object | None | Every object is reached through one | The JVM's garbage collector | Shares |

The table splits into two camps. In C, C++, Rust, and Go, a struct is its bytes, and
`b = a` gives you a second copy of them. In Python, JavaScript, TypeScript, Dart, and
Kotlin, a variable holds a reference to an object, and `b = a` gives you a second name
for the same object. That one difference explains most of the "copy or share?"
surprises in chapters 7 and 8.

---

## 9.3 Pointers and references

The first example takes an address, writes through it, walks along an array, and
measures what everything weighs.

**C**

```c
#include <stddef.h>
#include <stdio.h>

int main(void) {
    int x = 5;
    int *p = &x; // p holds the address of x
    printf("x = %d, *p = %d\n", x, *p);

    *p = 7; // writing through p changes x
    printf("after *p = 7: x = %d\n", x);

    // An array's name turns into a pointer to its first element.
    int scores[4] = {90, 85, 77, 68};
    int *q = scores;
    printf("q[2] = %d, *(q + 2) = %d\n", q[2], *(q + 2));

    // Adding 2 to an int pointer moves it 2 ints, which is 8 bytes.
    ptrdiff_t bytes = (char *)(q + 2) - (char *)q;
    printf("q + 2 is %td bytes past q\n", bytes);

    // sizeof on the array gives the whole array; on the pointer, just the address.
    printf("sizeof: x %zu, p %zu, scores %zu, q %zu\n", sizeof x, sizeof p,
           sizeof scores, sizeof q);

    // A pointer to a pointer: pp holds the address of p.
    int **pp = &p;
    **pp = 9;
    printf("after **pp = 9: x = %d\n", x);

    int *nowhere = NULL;
    printf("nowhere %s\n", nowhere == NULL ? "is NULL" : "points somewhere");
    return 0;
}
```

**C++**

```cpp
#include <array>
#include <cstddef>
#include <iostream>

int main() {
    int x = 5;
    int *p = &x; // a pointer, exactly as in C
    *p = 7;
    std::cout << "after *p = 7: x = " << x << "\n";

    // A reference is another name for x. It must be set when it is made,
    // and it can never be made to refer to anything else.
    int &r = x;
    r = 9;
    std::cout << "after r = 9: x = " << x << "\n";

    int y = 1;
    r = y; // copies y's value into x; r still names x
    y = 2;
    std::cout << "after r = y, then y = 2: x = " << x << ", y = " << y << "\n";

    // std::array is a C array that does not turn into a pointer.
    std::array<int, 4> scores = {90, 85, 77, 68};
    int *q = scores.data();
    std::cout << "q[2] = " << q[2] << ", *(q + 2) = " << *(q + 2) << "\n";

    std::cout << "sizeof: x " << sizeof x << ", p " << sizeof p << ", r " << sizeof r
              << ", scores " << sizeof scores << "\n";

    // nullptr has its own type, so it cannot be mistaken for the number 0.
    int *nowhere = nullptr;
    std::cout << "nowhere " << (nowhere == nullptr ? "is nullptr" : "points somewhere")
              << "\n";
}
```

**Rust**

```rust
fn main() {
    let mut x = 5;
    let r = &mut x; // a mutable reference: the only way to change x while r is in use
    *r = 7;
    println!("after *r = 7: x = {x}");

    // Shared references can only read, and any number of them may exist at once.
    let a = &x;
    let b = &x;
    println!("*a + *b = {}", *a + *b);

    // A slice is a pointer plus a length, so it knows where it ends.
    let scores = [90, 85, 77, 68];
    let middle = &scores[1..3];
    println!("middle = {middle:?}, middle[1] = {}", middle[1]);

    println!(
        "size_of_val: x {}, a {}, scores {}, middle {}",
        size_of_val(&x),
        size_of_val(&a),
        size_of_val(&scores),
        size_of_val(&middle)
    );

    // Raw pointers exist too. Making one is safe; reading through one is not.
    let p = &raw const x;
    let value = unsafe { *p };
    println!("through a raw pointer: {value}");

    // References are never null. "Maybe no reference" is an Option (chapter 8).
    let nowhere: Option<&i32> = None;
    println!("nowhere is {nowhere:?}");
}
```

*C:*

```text
x = 5, *p = 5
after *p = 7: x = 7
q[2] = 77, *(q + 2) = 77
q + 2 is 8 bytes past q
sizeof: x 4, p 8, scores 16, q 8
after **pp = 9: x = 9
nowhere is NULL
```

*C++:*

```text
after *p = 7: x = 7
after r = 9: x = 9
after r = y, then y = 2: x = 1, y = 2
q[2] = 77, *(q + 2) = 77
sizeof: x 4, p 8, r 4, scores 16
nowhere is nullptr
```

*Rust:*

```text
after *r = 7: x = 7
*a + *b = 14
middle = [85, 77], middle[1] = 77
size_of_val: x 4, a 8, scores 16, middle 16
through a raw pointer: 7
nowhere is None
```

**C: `&` takes an address and `*` follows one.** `int *p = &x` makes `p` hold the
address of `x`, and `*p = 7` writes 7 into whatever `p` points at, which is `x`. A
pointer to a pointer works the same way one level up: `**pp = 9` follows two addresses.
Pointer arithmetic counts in items, not bytes, so `q + 2` is 8 bytes past `q` when the
items are 4-byte `int`s, and `q[2]` is shorthand for `*(q + 2)`.

**An array is not a pointer, but it turns into one.** `sizeof scores` is 16, the whole
array, while `sizeof q` is 8, just an address. Almost anywhere else an array's name
stands for a pointer to its first item, which is why `int *q = scores` needs no `&`.
That pointer has forgotten the length, a fact that matters in section 9.5.

**C++ adds references.** `int &r = x` makes `r` another name for `x`. It must be bound
when it is made, and it can never be bound to anything else:

```text
error: 'r' declared as reference but not initialized
```

So `r = y` does not point `r` at `y`. It copies `y`'s value into `x`, and `y` changing
later has no effect. `sizeof r` is 4, the size of the `int` it names, because a
reference is not an object of its own. `std::array` is a C array that keeps its size
and does not turn into a pointer, and `nullptr` (C++11) is a null pointer with its own
type, unlike C's `NULL`, which may be just the number 0.

**Rust has two kinds of reference.** A shared reference `&x` can only read, and any
number of them may exist at once. A mutable reference `&mut x` can write, and while it
is in use it must be the only way to reach `x`. Section 9.6 shows why. References are
never null, so "maybe a reference" is `Option<&T>`, as in chapter 8. A reference
variable can be pointed elsewhere if it is declared `let mut r = &a;`, but a reference
never outlives what it points at, as section 9.4 shows.

Rust has no pointer arithmetic on references. A **slice**, such as `&scores[1..3]`, is
a pointer and a length together, which is why it takes 16 bytes. Every index is checked
against that length, so walking off the end panics instead of reading a neighbor's
memory. Raw pointers exist for talking to C (chapter 22): `&raw const x` (Rust 1.82)
makes one safely, but reading through it needs an `unsafe` block, because the compiler
can no longer promise that it points at anything.

| | C pointer | C++ pointer | C++ reference | Rust `&T`, `&mut T` | Rust raw pointer |
| --- | --- | --- | --- | --- | --- |
| Can be null | Yes | Yes | No | No | Yes |
| Can be pointed elsewhere | Yes | Yes | No | Yes, if the variable is `mut` | Yes |
| Arithmetic | Yes | Yes | No | No; slices instead | Yes |
| Checked to point at something still alive | No | No | No | Yes, at compile time | No |

---

## 9.4 The stack and the heap

The second example puts the same 1,000 ints on the stack and on the heap, and watches a
growable array reserve more room as it fills.

**C**

```c
#include <stdio.h>
#include <stdlib.h>

// Static data: one copy for the whole run, set up before main starts.
static int squares_made = 0;

// Each call gets its own stack frame with its own `here`. The frames stack up
// as the calls go deeper and are popped as each call returns.
static void countdown(int n) {
    int here = n * 10;
    if (n > 0) {
        countdown(n - 1);
    }
    printf(" %d", here);
}

// The squares go on the heap, so they outlive this call. The caller frees them.
static int *make_squares(size_t n) {
    int *squares = malloc(n * sizeof *squares);
    if (squares == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < n; i++) {
        squares[i] = (int)((i + 1) * (i + 1));
    }
    squares_made++;
    return squares;
}

int main(void) {
    printf("each frame keeps its own local:");
    countdown(3);
    printf("\n");

    int on_stack[1000] = {0}; // 4000 bytes inside main's frame
    printf("an array of 1000 ints on the stack: %zu bytes\n", sizeof on_stack);

    int *on_heap = make_squares(1000);
    if (on_heap == NULL) {
        return 1;
    }
    // sizeof sees only the pointer. The 4000 bytes it points to are elsewhere.
    printf("1000 squares on the heap: sizeof on_heap = %zu, on_heap[999] = %d\n",
           sizeof on_heap, on_heap[999]);
    free(on_heap); // give the memory back; on_heap now points at nothing usable
    on_heap = NULL;

    const char *literal = "hello"; // the characters are static data, read-only
    char copy[] = "hello";         // a writable copy of them on the stack
    copy[0] = 'j';
    printf("literal = %s, copy = %s, sizeof literal = %zu, sizeof copy = %zu\n", literal,
           copy, sizeof literal, sizeof copy);

    int *more = make_squares(3);
    if (more == NULL) {
        return 1;
    }
    printf("squares: %d %d %d\n", more[0], more[1], more[2]);
    free(more);
    printf("make_squares has run %d times\n", squares_made);
    return 0;
}
```

**C++**

```cpp
#include <array>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Each call gets its own stack frame with its own `here`.
void countdown(int n) {
    int here = n * 10;
    if (n > 0) {
        countdown(n - 1);
    }
    std::cout << " " << here;
}

int main() {
    std::cout << "each frame keeps its own local:";
    countdown(3);
    std::cout << "\n";

    std::array<int, 1000> on_stack{}; // 4000 bytes inside main's frame
    std::vector<int> on_heap(1000);   // a small header here, the items on the heap
    std::cout << "std::array of 1000 ints: " << sizeof on_stack << " bytes\n";
    std::cout << "std::vector of 1000 ints: " << sizeof on_heap << " bytes here, "
              << on_heap.size() * sizeof(int) << " on the heap\n";

    // The header holds a pointer to the items, the end of the items, and the end
    // of the space reserved for them. Adding past the end moves everything to a
    // bigger block on the heap.
    std::vector<int> growing;
    std::size_t capacity = growing.capacity();
    std::cout << "capacity as 100 items are added:";
    for (int i = 0; i < 100; i++) {
        growing.push_back(i);
        if (growing.capacity() != capacity) {
            capacity = growing.capacity();
            std::cout << " " << capacity;
        }
    }
    std::cout << "\nsizeof growing is still " << sizeof growing << "\n";

    // make_unique puts one value on the heap and frees it when `boxed` goes away
    // (chapter 10).
    auto boxed = std::make_unique<int>(42);
    std::cout << "*boxed = " << *boxed << ", sizeof boxed = " << sizeof boxed << "\n";

    std::string word = "hello";
    std::cout << "sizeof a std::string: " << sizeof word << "\n";
}
```

**Rust**

```rust
// Static data: one copy for the whole run.
static GREETING: &str = "hello";

// Each call gets its own stack frame with its own `here`.
fn countdown(n: i32) {
    let here = n * 10;
    if n > 0 {
        countdown(n - 1);
    }
    print!(" {here}");
}

fn main() {
    print!("each frame keeps its own local:");
    countdown(3);
    println!();

    let on_stack = [0i32; 1000]; // 4000 bytes inside main's frame
    let on_heap = vec![0i32; 1000]; // a small header here, the items on the heap
    println!("[i32; 1000]: {} bytes", size_of_val(&on_stack));
    println!(
        "Vec<i32> of 1000: {} bytes here, {} on the heap",
        size_of_val(&on_heap),
        on_heap.len() * size_of::<i32>()
    );

    // The header holds a pointer, the capacity, and the length. Pushing past the
    // capacity moves the items to a bigger block on the heap.
    let mut growing = Vec::new();
    let mut capacity = growing.capacity();
    print!("capacity as 100 items are added:");
    for i in 0..100 {
        growing.push(i);
        if growing.capacity() != capacity {
            capacity = growing.capacity();
            print!(" {capacity}");
        }
    }
    println!();
    println!("size_of_val(&growing) is still {}", size_of_val(&growing));

    // Box puts one value on the heap and frees it when `boxed` goes away (chapter 10).
    let boxed = Box::new(42);
    let size = size_of_val(&boxed);
    println!("*boxed = {}, size_of_val(&boxed) = {size}", *boxed);

    // A String owns heap memory; a &str can point into static data instead.
    let owned = String::from(GREETING);
    println!(
        "String: {} bytes, &str: {} bytes, both say {owned}",
        size_of_val(&owned),
        size_of_val(&GREETING)
    );
}
```

*C:*

```text
each frame keeps its own local: 0 10 20 30
an array of 1000 ints on the stack: 4000 bytes
1000 squares on the heap: sizeof on_heap = 8, on_heap[999] = 1000000
literal = hello, copy = jello, sizeof literal = 8, sizeof copy = 6
squares: 1 4 9
make_squares has run 2 times
```

*C++:*

```text
each frame keeps its own local: 0 10 20 30
std::array of 1000 ints: 4000 bytes
std::vector of 1000 ints: 24 bytes here, 4000 on the heap
capacity as 100 items are added: 1 2 4 8 16 32 64 128
sizeof growing is still 24
*boxed = 42, sizeof boxed = 8
sizeof a std::string: 32
```

*Rust:*

```text
each frame keeps its own local: 0 10 20 30
[i32; 1000]: 4000 bytes
Vec<i32> of 1000: 24 bytes here, 4000 on the heap
capacity as 100 items are added: 4 8 16 32 64 128
size_of_val(&growing) is still 24
*boxed = 42, size_of_val(&boxed) = 8
String: 24 bytes, &str: 16 bytes, both say hello
```

**Each call has its own frame.** `countdown(3)` calls `countdown(2)`, which calls
`countdown(1)` and `countdown(0)`. Four frames are on the stack at once, each with its
own `here`, and they print in reverse as each call returns and its frame is popped.

**Arrays live where they are declared.** `int on_stack[1000]` is 4,000 bytes inside
`main`'s frame, and so are C++'s `std::array<int, 1000>` and Rust's `[i32; 1000]`. A
local array of a million `int`s would need 4 MB of an 8 MiB stack, so big or growing
data goes on the heap.

**C asks for heap memory by hand.** `malloc(n * sizeof *squares)` returns a pointer to
a fresh block, or `NULL` if there is none, and the block stays until `free` gives it
back. That is why `make_squares` can return it: the block does not belong to the
function's frame. `sizeof on_heap` is 8, because `sizeof` sees only the pointer, never
how much memory it points to. Chapter 10 is about making sure every block is freed
exactly once.

**String literals are static data.** `"hello"` is stored once for the whole program,
usually in read-only memory, so `literal` points at bytes that must not change. Writing
through a `char *` to a literal compiles in C, and on Linux the program dies with
`Segmentation fault` when it runs. C++11 made a plain `char *` to a literal illegal, and
GCC says so: `ISO C++ forbids converting a string constant to 'char*'`.
`char copy[] = "hello"` is different: it is a 6-byte array on the stack, counting the
`'\0'` at the end, filled with a copy of the literal, and it can be changed.

**A vector is a small header on the stack and its items on the heap.** A
`std::vector<int>` and a Rust `Vec<i32>` are both 24 bytes however many items they hold:
three 8-byte words that record where the items are, how many there are, and how many
fit, which is called the **capacity**. Pushing past the capacity gets a bigger block,
and the items usually move to a new address. GCC's C++ library doubles the capacity each
time, starting at 1, and Rust's current standard library starts a `Vec<i32>` at 4 and
doubles too. Neither growth pattern is promised by the documentation. Exercise 9.3
builds the same thing in C.

**`std::string` and `String` follow the same plan.** Rust's `String` is a 24-byte
header like `Vec`'s, while a `&str` is a 16-byte pointer and length that can point into
static data. With GCC's library a `std::string` is 32 bytes, because it keeps up to 15
characters inside the header itself, with no heap block at all. This is called the
**small string optimization**, and other libraries pick other sizes.

**Rust's `Box` and C++'s `std::make_unique` put one value on the heap** and free it
when the variable goes away. Both are 8 bytes: one pointer. They are chapter 10's
subject.

### The stack is small

Run a recursive function with no base case and each language reaches the end of the
stack differently. The probe was `depth(n) = 1 + depth(n + 1)`:

| Language | What happens |
| --- | --- |
| C, C++ | `Segmentation fault`: the program is killed |
| Rust | `thread 'main' (12236) has overflowed its stack`, where 12236 is the thread's ID, then `fatal runtime error: stack overflow, aborting` |
| Go | `runtime: goroutine stack exceeds 1000000000-byte limit`, then `fatal error: stack overflow` |
| Python | `RecursionError: maximum recursion depth exceeded`, after 1,000 frames by default |
| JavaScript, TypeScript | `RangeError: Maximum call stack size exceeded` |
| Dart | `Unhandled exception:` and then `Stack Overflow` |
| Kotlin | `Exception in thread "main" java.lang.StackOverflowError` |

Go is the odd one. Each goroutine's stack starts at 2 KB and grows by copying itself to
a bigger block when needed, up to 1 GB on 64-bit systems, so deep recursion that would
crash C works fine in Go. Python counts frames rather than bytes, and
`sys.setrecursionlimit` changes the limit. Chapter 29 turns deep recursion into loops
when it matters.

### Pointers that outlive their memory

A local lives in its function's frame, so a pointer to it is useless once the function
returns. The frame's memory will be reused by the next call, and anything read through
the old pointer is whatever happens to be there. Such a pointer is said to **dangle**.

```c
int *broken(void) {
    int x = 5;
    return &x;
}
```

GCC warns, `function returns address of local variable [-Wreturn-local-addr]`, and then
makes the function return `NULL`, so a careless caller crashes at once instead of
reading stale memory later. C++ gives the same warning for a reference, `reference to
local variable 'x' returned`. Both are only warnings: without `-Werror` the program
builds.

Rust will not build it. The plain version fails before the borrow checker even looks,
because a returned reference must say what it borrows from:

```text
error[E0106]: missing lifetime specifier
```

Promising that it lives forever, with `-> &'static i32`, gets the real reason:

```text
error[E0515]: cannot return reference to local variable `x`
```

A raw pointer can still dangle, because raw pointers are not checked. Since Rust 1.91
the compiler warns about the plain case, `function returns a dangling pointer to
dropped local variable 'x'`, from the `dangling_pointers_from_locals` lint, and notes
that making the pointer is safe but reading through it is undefined behavior.

**Growing a vector leaves pointers into it dangling.** This C++ compiles without a
warning:

```cpp
std::vector<int> v = {1, 2, 3};
int &first = v[0];
v.push_back(4); // the items move to a bigger block
std::cout << first << "\n";
```

`push_back` moved the items to a new block and freed the old one, so `first` refers to
freed memory. One run printed `1604056377`. Built with `-fsanitize=address`, the same
program stops with `AddressSanitizer: heap-use-after-free`. The Rust version does not
compile:

```text
error[E0502]: cannot borrow `v` as mutable because it is also borrowed as immutable
```

`first` borrows `v`, `push` needs to borrow it mutably, and the two may not overlap.
This is the same rule as in section 9.6, and it is why Rust programs do not have this
bug.

---

## 9.5 Passing arguments

When you call a function, each argument is copied into the new frame. What is copied
decides what the function can change.

**C**

```c
#include <stddef.h>
#include <stdio.h>

typedef struct {
    int x;
    int y;
} Point;

// C passes every argument by value: the function gets its own copy.
static void bump_copy(int n) {
    n++;
    printf("inside bump_copy: n = %d\n", n);
}

// To change the caller's variable, pass its address.
static void bump(int *n) {
    (*n)++;
}

// An array parameter is really a pointer to the first item, so the function
// changes the caller's array. It cannot tell how long the array is, either.
static void double_all(int values[], size_t len) {
    for (size_t i = 0; i < len; i++) {
        values[i] *= 2;
    }
}

static void shift_copy(Point p) {
    p.x += 10;
    printf("inside shift_copy: p = (%d, %d)\n", p.x, p.y);
}

static void shift(Point *p) {
    p->x += 10; // p->x means (*p).x
}

// A pointer to const reads the caller's struct without copying it, and the
// compiler refuses any write through it.
static int sum(const Point *p) {
    return p->x + p->y;
}

int main(void) {
    int n = 1;
    bump_copy(n);
    printf("after bump_copy(n): n = %d\n", n);
    bump(&n);
    printf("after bump(&n): n = %d\n", n);

    int values[3] = {1, 2, 3};
    double_all(values, 3);
    printf("after double_all(values, 3): %d %d %d\n", values[0], values[1], values[2]);

    Point p = {1, 2};
    shift_copy(p);
    printf("after shift_copy(p): p = (%d, %d)\n", p.x, p.y);
    shift(&p);
    printf("after shift(&p): p = (%d, %d)\n", p.x, p.y);
    printf("sum(&p) = %d\n", sum(&p));
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <vector>

void bump_copy(int n) {
    n++;
    std::cout << "inside bump_copy: n = " << n << "\n";
}

// A reference parameter is another name for the caller's variable.
void bump(int &n) {
    n++;
}

// The C way still works.
void bump_pointer(int *n) {
    (*n)++;
}

// Passing a vector by value copies all of its items.
void add_copy(std::vector<int> v) {
    v.push_back(4);
    std::cout << "inside add_copy: " << v.size() << " items\n";
}

void add(std::vector<int> &v) {
    v.push_back(4);
}

// A const reference reads the caller's vector without copying it.
int total(const std::vector<int> &v) {
    int sum = 0;
    for (int item : v) {
        sum += item;
    }
    return sum;
}

int main() {
    int n = 1;
    bump_copy(n);
    std::cout << "after bump_copy(n): n = " << n << "\n";
    bump(n); // looks just like the call above, but changes n
    std::cout << "after bump(n): n = " << n << "\n";
    bump_pointer(&n);
    std::cout << "after bump_pointer(&n): n = " << n << "\n";

    std::vector<int> v = {1, 2, 3};
    add_copy(v);
    std::cout << "after add_copy(v): v has " << v.size() << " items\n";
    add(v);
    std::cout << "after add(v): v has " << v.size() << " items\n";
    std::cout << "total(v) = " << total(v) << "\n";
}
```

**Rust**

```rust
fn bump_copy(mut n: i32) {
    n += 1;
    println!("inside bump_copy: n = {n}");
}

// A mutable reference lets the function change the caller's variable.
fn bump(n: &mut i32) {
    *n += 1;
}

// Passing a Vec by value hands it over (chapter 10), so the caller clones it
// first if it wants to keep its own.
fn add_copy(mut v: Vec<i32>) {
    v.push(4);
    println!("inside add_copy: {} items", v.len());
}

fn add(v: &mut Vec<i32>) {
    v.push(4);
}

// A shared reference to a slice reads the items without copying them.
fn total(v: &[i32]) -> i32 {
    v.iter().sum()
}

fn main() {
    let mut n = 1;
    bump_copy(n);
    println!("after bump_copy(n): n = {n}");
    bump(&mut n); // the call says that n may change
    println!("after bump(&mut n): n = {n}");

    let mut v = vec![1, 2, 3];
    add_copy(v.clone());
    println!("after add_copy(v.clone()): v has {} items", v.len());
    add(&mut v);
    println!("after add(&mut v): v has {} items", v.len());
    println!("total(&v) = {}", total(&v));
}
```

*C:*

```text
inside bump_copy: n = 2
after bump_copy(n): n = 1
after bump(&n): n = 2
after double_all(values, 3): 2 4 6
inside shift_copy: p = (11, 2)
after shift_copy(p): p = (1, 2)
after shift(&p): p = (11, 2)
sum(&p) = 13
```

*C++:*

```text
inside bump_copy: n = 2
after bump_copy(n): n = 1
after bump(n): n = 2
after bump_pointer(&n): n = 3
inside add_copy: 4 items
after add_copy(v): v has 3 items
after add(v): v has 4 items
total(v) = 10
```

*Rust:*

```text
inside bump_copy: n = 2
after bump_copy(n): n = 1
after bump(&mut n): n = 2
inside add_copy: 4 items
after add_copy(v.clone()): v has 3 items
after add(&mut v): v has 4 items
total(&v) = 10
```

**C passes everything by value.** `bump_copy(n)` gets its own `n`, so its change dies
with its frame. To change the caller's variable, the caller passes its address and the
function writes through it: `bump(&n)`. Passing a struct copies the whole struct, so
`shift_copy` shifts its own copy of the point, while `shift(&p)` shifts the caller's. A
`const Point *` parameter reads a big struct without copying it, and the compiler
refuses any write through it.

**Arrays are the exception, because they turn into pointers.** `double_all(values, 3)`
receives the address of the first item, so it doubles the caller's numbers. The
parameter `int values[]` is really `int *values`, which is why the length is passed
separately. GCC catches the classic mistake of asking the parameter for its size:

```text
warning: 'sizeof' on array function parameter 'values' will return size of 'int *' [-Wsizeof-array-argument]
```

**C++ adds reference parameters.** `void bump(int &n)` makes `n` another name for the
caller's variable, so `bump(n)` changes it. The cost is that the call site no longer
tells you: `bump_copy(n)` and `bump(n)` look identical, and only the declaration says
which one changes `n`. Containers have value semantics, so passing a `std::vector` by
value copies every item, as `add_copy` shows. The usual signatures are `const T &` for
a big value the function only reads, `T &` for one it changes, and plain `T` for small
values or when the function wants its own copy.

**Rust says it at the call site.** `bump(&mut n)` is visibly different from
`bump_copy(n)`. A function that reads a list takes `&[i32]`, which accepts a `Vec`, an
array, or part of either, and one that changes it takes `&mut Vec<i32>`. Passing a
`Vec` by value does not copy it. It **moves** it: the function now owns the vector, and
the caller cannot use it again unless it handed over a `clone()` instead. Chapter 10
explains moves.

| Goal | C | C++ | Rust |
| --- | --- | --- | --- |
| Change the caller's variable | `T *` | `T &` | `&mut T` |
| Read a big value without copying | `const T *` | `const T &` | `&T`, or `&[T]` for a list |
| Give the function its own copy | `T`; arrays need a struct or `memcpy` | `T` | `T` for `Copy` types, otherwise pass `x.clone()` |

---

## 9.6 Aliasing

Aliasing means two names for one place. The example shows two bugs it causes and how
each language deals with them.

**C**

```c
#include <stddef.h>
#include <stdio.h>

// Swaps two ints without a temporary. It works only while a and b point to
// different places.
static void xor_swap(int *a, int *b) {
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

// Adds *amount to every value. If amount points into values, the amount
// changes partway through the loop.
static void add_each(int *values, size_t len, const int *amount) {
    for (size_t i = 0; i < len; i++) {
        values[i] += *amount;
    }
}

int main(void) {
    int x = 1;
    int *a = &x;
    int *b = &x; // a and b are aliases: two names for one int
    *a = 5;
    printf("after *a = 5: *b = %d\n", *b);

    int y = 3;
    xor_swap(&x, &y);
    printf("after xor_swap(&x, &y): x = %d, y = %d\n", x, y);
    xor_swap(&x, &x);
    printf("after xor_swap(&x, &x): x = %d\n", x);

    int values[3] = {1, 2, 3};
    add_each(values, 3, &values[0]);
    printf("add_each(values, 3, &values[0]): %d %d %d\n", values[0], values[1],
           values[2]);

    int fixed[3] = {1, 2, 3};
    int amount = fixed[0]; // a copy, so it cannot change under the loop
    add_each(fixed, 3, &amount);
    printf("with a copy of the amount: %d %d %d\n", fixed[0], fixed[1], fixed[2]);
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <string>
#include <utility>
#include <vector>

std::string show(const std::vector<int> &v) {
    std::string out;
    for (int item : v) {
        out += (out.empty() ? "" : " ") + std::to_string(item);
    }
    return out;
}

// const means this function will not change amount. It does not mean that
// nothing else will.
void add_each(std::vector<int> &values, const int &amount) {
    for (int &item : values) {
        item += amount;
    }
}

int main() {
    int x = 1;
    int &a = x;
    int &b = x; // two references to one int
    a = 5;
    std::cout << "after a = 5: b = " << b << "\n";

    int y = 3;
    std::swap(x, y); // uses a temporary, so it is safe even for std::swap(x, x)
    std::cout << "after std::swap(x, y): x = " << x << ", y = " << y << "\n";

    std::vector<int> values = {1, 2, 3};
    add_each(values, values[0]); // amount is values[0] itself
    std::cout << "add_each(values, values[0]): " << show(values) << "\n";

    std::vector<int> fixed = {1, 2, 3};
    int amount = fixed[0];
    add_each(fixed, amount);
    std::cout << "with a copy of the amount: " << show(fixed) << "\n";

    // Two vectors are two separate blocks of memory, even after assignment.
    std::vector<int> other = fixed;
    other[0] = 100;
    std::cout << "other = fixed, then other[0] = 100: fixed = " << show(fixed) << "\n";
}
```

**Rust**

```rust
use std::mem;

fn add_each(values: &mut [i32], amount: i32) {
    for item in values.iter_mut() {
        *item += amount;
    }
}

fn main() {
    let mut x = 1;
    let a = &x;
    let b = &x; // many shared references may alias, because none can write
    println!("two shared references: *a = {a}, *b = {b}");

    // A &mut must be the only way to reach x while it is in use.
    let only = &mut x;
    *only = 5;
    println!("after *only = 5: x = {x}");

    let mut y = 3;
    mem::swap(&mut x, &mut y);
    println!("after mem::swap(&mut x, &mut y): x = {x}, y = {y}");

    // mem::swap(&mut values[0], &mut values[2]) is refused: two &mut into one
    // Vec at once. The Vec's own swap method takes indexes instead.
    let mut values = vec![1, 2, 3];
    values.swap(0, 2);
    println!("after values.swap(0, 2): {values:?}");

    // add_each(&mut values, &values[0]) is refused too, so the amount is copied.
    let amount = values[0];
    add_each(&mut values, amount);
    println!("add_each(&mut values, values[0]): {values:?}");

    // split_at_mut proves to the compiler that two &mut do not overlap.
    let (left, right) = values.split_at_mut(1);
    mem::swap(&mut left[0], &mut right[1]);
    println!("after swapping across split_at_mut(1): {values:?}");
}
```

*C:*

```text
after *a = 5: *b = 5
after xor_swap(&x, &y): x = 3, y = 5
after xor_swap(&x, &x): x = 0
add_each(values, 3, &values[0]): 2 4 5
with a copy of the amount: 2 3 4
```

*C++:*

```text
after a = 5: b = 5
after std::swap(x, y): x = 3, y = 5
add_each(values, values[0]): 2 4 5
with a copy of the amount: 2 3 4
other = fixed, then other[0] = 100: fixed = 2 3 4
```

*Rust:*

```text
two shared references: *a = 1, *b = 1
after *only = 5: x = 5
after mem::swap(&mut x, &mut y): x = 3, y = 5
after values.swap(0, 2): [3, 2, 1]
add_each(&mut values, values[0]): [6, 5, 4]
after swapping across split_at_mut(1): [4, 5, 6]
```

**The XOR swap breaks on aliases.** Exchanging two numbers with three XORs needs no
temporary, and it works for `&x, &y`. But when both pointers name the same `int`, the
first line computes `x ^ x`, which is 0, and the value is gone. A swap through a
temporary is correct even when both names are one place, and `std::swap` works that
way. Rust's `mem::swap` cannot be handed one place twice at all, because that would
take two `&mut` to it.

**A read-only parameter can still change.** `add_each(values, 3, &values[0])` asks to
add the first value to every value. After the first step `values[0]` is 2, and
`amount` points at it, so the rest of the loop adds 2 instead of 1, giving `2 4 5`
rather than `2 3 4`. The `const` in `const int *amount` only promises that `add_each`
will not write through `amount`. It says nothing about other names for the same
memory, and here `values` is one. C++'s `const int &amount` has the same hole. The fix
in every language is the one the examples use: copy the value first, so nothing can
change it.

**Compilers have to assume the worst.** Because `amount` might alias `values`, the
compiler must reload `*amount` from memory after every write to `values[i]`, which
slows the loop down. C99 added `restrict` for this. Declaring `int *restrict values`
and `const int *restrict amount` promises that, during the call, the memory each one
points at is reached through it alone, and the compiler may then keep `*amount` in a
register. If the promise is false, the behavior is undefined. GCC's
`-Wrestrict`, part of `-Wall`, catches only the plainest case, the same pointer passed
for two `restrict` parameters.

**Rust makes the rule part of the language: many readers, or one writer.** At any
moment a value can have any number of `&` references or exactly one `&mut`, never
both. Two `&x` alias happily because neither can write. The aliasing call from the C
version does not compile:

```text
error[E0502]: cannot borrow `values` as immutable because it is also borrowed as mutable
```

So the Rust example copies `values[0]` into `amount` first, which is the fixed C code
written the only way Rust allows. The same rule refuses
`mem::swap(&mut values[0], &mut values[2])`, with
`error[E0499]: cannot borrow 'values' as mutable more than once at a time`, because the
compiler does not compare the indexes. `values.swap(0, 2)` does the job, and
`split_at_mut(1)` splits a slice into two halves that cannot overlap, so a `&mut` into
each is allowed. Because a `&mut` is never aliased, the Rust compiler can make the same
assumption that `restrict` allows in C, without being told. Shared, changeable state is
still possible through `Cell`, `RefCell`, and `Mutex`, which check the rule in other
ways (chapters 10 and 16).

**Value semantics avoids aliasing in the first place.** The last line of the C++
example copies a vector, and changing the copy leaves the original alone. Two C++
containers never share their items unless you made them share, through a pointer, a
reference, or a view such as `std::span`.

---

## 9.7 Shallow and deep copies

Copying a value that holds a pointer raises the question of whether the copy gets its
own pointed-to data.

**C**

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The characters are inside the struct, so copying the struct copies them.
typedef struct {
    char name[16];
    int score;
} Inline;

// Only an address is inside the struct. Copying the struct copies the address,
// so both copies share one set of characters: a shallow copy.
typedef struct {
    char *name;
    int score;
} Pointing;

// A new heap copy of a string, or NULL if there is no memory.
static char *copy_string(const char *s) {
    size_t size = strlen(s) + 1; // + 1 for the '\0' at the end
    char *copy = malloc(size);
    if (copy != NULL) {
        memcpy(copy, s, size);
    }
    return copy;
}

int main(void) {
    Inline a = {"Ada", 90};
    Inline b = a;
    b.name[0] = 'E';
    b.score = 70;
    printf("inline array: a = %s %d, b = %s %d\n", a.name, a.score, b.name, b.score);

    Pointing c = {copy_string("Ada"), 90};
    if (c.name == NULL) {
        return 1;
    }
    Pointing d = c; // copies the pointer, not the characters
    d.name[0] = 'E';
    d.score = 70;
    printf("shallow copy: c = %s %d, d = %s %d\n", c.name, c.score, d.name, d.score);

    // A deep copy gives the new struct characters of its own.
    Pointing e = c;
    e.name = copy_string(c.name);
    if (e.name == NULL) {
        return 1;
    }
    e.name[0] = 'I';
    printf("deep copy: c = %s, e = %s\n", c.name, e.name);

    free(e.name);
    free(c.name); // c and d shared this block, so it is freed once, not twice
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <string>
#include <vector>

struct Player {
    std::string name;
    std::vector<int> scores;
};

std::string show(const Player &p) {
    std::string out = p.name;
    for (int score : p.scores) {
        out += " " + std::to_string(score);
    }
    return out;
}

// A raw pointer member is copied as an address, just as in C.
struct Counter {
    int *count;
};

int main() {
    // Copying a struct copies each member, and std::string and std::vector
    // copy their own heap memory, so the copy is deep.
    Player a{"Ada", {90, 85}};
    Player b = a;
    b.name[0] = 'E';
    b.scores.push_back(70);
    std::cout << "b = a copies everything: a = " << show(a) << ", b = " << show(b)
              << "\n";

    int shared = 1;
    Counter c{&shared};
    Counter d = c;
    *d.count = 2;
    std::cout << "a copied pointer still points at the same int: *c.count = "
              << *c.count << "\n";

    std::vector<Player> team = {a, b};
    for (auto p : team) { // p is a copy of each player
        p.name += "?";
    }
    std::cout << "for (auto p : team) changes copies: " << team[0].name << " "
              << team[1].name << "\n";
    for (auto &p : team) { // p refers to each player
        p.name += "!";
    }
    std::cout << "for (auto &p : team) changes the players: " << team[0].name << " "
              << team[1].name << "\n";
}
```

**Rust**

```rust
#[derive(Clone)]
struct Player {
    name: String,
    scores: Vec<i32>,
}

#[derive(Clone, Copy)]
struct Point {
    x: i32,
    y: i32,
}

fn main() {
    // clone() copies every field, and String and Vec clone their heap memory,
    // so the copy is deep.
    let a = Player {
        name: String::from("Ada"),
        scores: vec![90, 85],
    };
    let mut b = a.clone();
    b.name.replace_range(0..1, "E");
    b.scores.push(70);
    println!(
        "a.clone() copies everything: a = {} {:?}, b = {} {:?}",
        a.name, a.scores, b.name, b.scores
    );

    // Plain assignment moves a Player instead of copying it. The compiler then
    // refuses any use of `a` (chapter 10).
    let c = a;
    println!("let c = a moves it: c.name = {}", c.name);

    // A Copy type is copied bit for bit on assignment, like a C struct.
    let p = Point { x: 1, y: 2 };
    let mut q = p;
    q.x = 5;
    println!(
        "let q = p copies a Copy type: p = ({}, {}), q = ({}, {})",
        p.x, p.y, q.x, q.y
    );

    let mut team = vec![c, b];
    for player in team.iter_mut() {
        player.name.push('!');
    }
    let names: Vec<&str> = team.iter().map(|player| player.name.as_str()).collect();
    println!("iter_mut changes the players: {names:?}");
}
```

*C:*

```text
inline array: a = Ada 90, b = Eda 70
shallow copy: c = Eda 90, d = Eda 70
deep copy: c = Eda, e = Ida
```

*C++:*

```text
b = a copies everything: a = Ada 90 85, b = Eda 90 85 70
a copied pointer still points at the same int: *c.count = 2
for (auto p : team) changes copies: Ada Eda
for (auto &p : team) changes the players: Ada! Eda!
```

*Rust:*

```text
a.clone() copies everything: a = Ada [90, 85], b = Eda [90, 85, 70]
let c = a moves it: c.name = Ada
let q = p copies a Copy type: p = (1, 2), q = (5, 2)
iter_mut changes the players: ["Ada!", "Eda!"]
```

**C copies bytes and nothing more.** `Inline b = a` copies the struct's 20 bytes,
including the whole `name` array, so `b` has its own characters. `Pointing d = c`
copies the struct's bytes too, but those bytes hold an address, so `c` and `d` share one
heap block, and changing `d.name[0]` changes `c`'s name. That is a **shallow copy**. A
**deep copy** has to be written by hand, as `copy_string` does for `e`. Shallow copies
also make freeing tricky: `c` and `d` share one block, so it must be freed once. Freeing
it through both is a double free, which glibc on Linux stops with `free(): double free
detected in tcache 2`. Odd but useful: C will not assign one array to another (`error:
assignment to expression with array type`), yet wrapping the array in a struct makes it
copy.

**C++ copies each member with that member's own copy.** `std::string` and
`std::vector` copy their heap memory, so `Player b = a` is a deep copy, with no code
written. A raw pointer member is copied as an address, as in C, so `Counter d = c`
shares the `int`. Chapter 10 shows how a class takes control of its own copying.

**C++'s quiet copies.** Value semantics means copies happen wherever a type is named by
value. `for (auto p : team)` copies each player, so changing `p` changes nothing, while
`for (auto &p : team)` refers to each one. The same goes for `Player first = team[0]`
against `Player &first = team[0]`. GCC does not warn about either.

**Rust copies only when asked.** `a.clone()` makes a deep copy, because `String` and
`Vec` clone their heap memory. Plain assignment `let c = a` moves the player instead, so
`a` cannot be used afterward:

```text
error[E0382]: borrow of moved value: `a`
```

A type made only of plain values can derive `Copy`, and then assignment copies its bits
like a C struct, as `Point` does. Rust refuses `Copy` for a type holding a `String`,
since copying the bits would make two owners of one heap block (chapter 8). So a Rust
copy is either cheap and implicit, or visible as `.clone()`. `iter_mut()` hands out
`&mut` references to each item, the equivalent of C++'s `auto &`.

---

## 9.8 The garbage-collected languages

Python, JavaScript, TypeScript, Dart, and Kotlin share one memory model. Every object
lives on the heap, a variable holds a reference to it, and a garbage collector frees an
object once nothing refers to it. There is no `&`, no `*`, and no `free`. Numbers and
strings cannot be changed, so sharing them is never visible, and the questions of
this chapter come down to lists, maps, and objects. Go has a garbage collector too, but
it also has value types and pointers, so it gets its own part below.

**Python**

```python
import copy


def add_five(items: list[int]) -> None:
    items.append(5)  # changes the caller's list, because items refers to it


def yes_no(flag: bool) -> str:
    return "yes" if flag else "no"


def main() -> None:
    # Every variable holds a reference. Assignment copies the reference.
    a = [1, 2, 3]
    b = a
    b.append(4)
    print(f"b = a, then b gets 4: a = {a}")

    add_five(a)
    print(f"a function appends 5: a = {a}")

    # Numbers and strings cannot change, so sharing them is never visible.
    n = 1
    m = n
    m += 1  # makes a new int and points m at it
    print(f"m = n, then m += 1: n = {n}, m = {m}")

    # A shallow copy is a new outer list holding the same inner lists.
    grid = [[1, 2], [3, 4]]
    shallow = grid.copy()
    shallow[0][0] = 9
    print(f"shallow copy, then copy[0][0] = 9: grid = {grid}")

    deep = copy.deepcopy(grid)
    deep[0][0] = 7
    print(f"deep copy, then copy[0][0] = 7: grid = {grid}")

    # * repeats the reference, so all three rows are one list.
    rows = [[0, 0]] * 3
    rows[0][0] = 1
    print(f"one row in 3 slots, then rows[0][0] = 1: rows = {rows}")

    # `is` asks "same object?"; == asks "equal items?".
    same = a.copy()
    print(f"a and b are the same list: {yes_no(a is b)}")
    print(f"a and its copy are the same list: {yes_no(a is same)}")
    print(f"a and its copy hold equal items: {yes_no(a == same)}")


if __name__ == "__main__":
    main()
```

The JavaScript, TypeScript, Dart, and Kotlin versions in
[examples/06-references](examples/06-references/) print exactly the same lines:

*Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
b = a, then b gets 4: a = [1, 2, 3, 4]
a function appends 5: a = [1, 2, 3, 4, 5]
m = n, then m += 1: n = 1, m = 2
shallow copy, then copy[0][0] = 9: grid = [[9, 2], [3, 4]]
deep copy, then copy[0][0] = 7: grid = [[9, 2], [3, 4]]
one row in 3 slots, then rows[0][0] = 1: rows = [[1, 0], [1, 0], [1, 0]]
a and b are the same list: yes
a and its copy are the same list: no
a and its copy hold equal items: yes
```

**Assignment shares, and so does passing.** `b = a` copies the reference, so `b.append`
changes the one list both names refer to. Passing `a` to `add_five` copies the
reference into the parameter, so the function can change the caller's list too. What a
function cannot do is change which object the caller's variable refers to. Assigning to
the parameter only repoints the parameter:

```python
def replace(items: list[int]) -> None:
    items = [0]  # repoints the local name; the caller's list is untouched
    items.append(6)


a = [1, 2, 3]
replace(a)
print(a)  # [1, 2, 3]
```

This is sometimes called **call by sharing**. Python, JavaScript, TypeScript, and Dart
allow assigning to a parameter, but the effect stays inside the function. Kotlin forbids
it, since parameters are read-only: `'val' cannot be reassigned`. It also explains why
exercise 9.1's swap cannot be written for two plain variables in these languages.

**The rows trap.** `[[0, 0]] * 3` repeats one reference three times, so all three rows
are the same list, and writing `rows[0][0]` shows up in every row. Each language has its
own way to fall into it, and its own way out:

| | Python | JavaScript, TypeScript | Dart | Kotlin |
| --- | --- | --- | --- | --- |
| One row in every slot | `[[0] * w] * h` | `Array(h).fill(row)` | `List.filled(h, row)` | `MutableList(h) { row }` |
| A new row in every slot | `[[0] * w for _ in range(h)]` | `Array.from({ length: h }, () => Array(w).fill(0))` | `List.generate(h, (_) => List.filled(w, 0))` | `MutableList(h) { MutableList(w) { 0 } }` |
| Same object? | `a is b` | `a === b` | `identical(a, b)` | `a === b` |
| Equal items? | `a == b` | No built-in; compare the items | No built-in; `==` on lists asks "same object?" | `a == b` |
| Shallow copy | `a.copy()`, `a[:]`, `list(a)` | `[...a]`, `a.slice()` | `List.of(a)`, `[...a]` | `a.toMutableList()` |
| Deep copy | `copy.deepcopy(a)` | `structuredClone(a)` | None built in | None built in |
| A list nobody can change | `tuple(a)` | `Object.freeze(a)`, shallow; TypeScript's `readonly T[]` at compile time | `List.unmodifiable(a)` | None built in; `List<T>` is a read-only view |

**Shallow copies share what is inside.** `grid.copy()`, `[...grid]`, `List.of(grid)`,
and `grid.toMutableList()` make a new outer list holding the same rows, so writing
`shallow[0][0]` changes `grid` too. Python's `copy.deepcopy` and JavaScript's
`structuredClone` (Node 17) copy everything they can reach. `structuredClone` refuses
functions (`DataCloneError: f() {} could not be cloned.`). Dart and Kotlin have no
general deep copy, so the examples copy each row by hand, which is also the clearest
way in Python and JavaScript. A Kotlin data class's `copy()` is shallow as well: a copy
of a team with a `MutableList` of scores shares the list.

**Read-only is not the same as immutable.** Kotlin's `List<T>` hides the methods that
change a list, but the object may still be a `MutableList` that its owner keeps
changing, and a cast gets the methods back. JavaScript's `Object.freeze` stops changes
to the object itself but not to the objects inside it, and TypeScript's `readonly`
exists only at compile time: `Property 'push' does not exist on type 'readonly
number[]'`. Python's tuple and Dart's `List.unmodifiable` really cannot change, though
a tuple of lists still holds changeable lists.

**Identity is rarely what you want.** `is`, `===`, and `identical` ask whether two
references point at the same object. For numbers and strings that answer depends on
how the runtime happens to store them. In CPython, `int("1000") is 1000` is `False`
while `int("5") is 5` is `True`, because small integers are cached, and Python warns
about the literal: `SyntaxWarning: "is" with 'int' literal. Did you mean "=="?`. Use
`is` only for `None`. Kotlin stores an `Int` as a plain number where it can, and as a
boxed object when it is nullable or inside a collection, so `===` on two `Int?` is
`false` for 1000 and is flagged: `identity equality for arguments of types 'Int?' and
'Int?' is prohibited`.

**When memory is freed.** CPython counts the references to each object and frees it the
moment the count reaches zero, and from time to time a separate cycle collector frees
groups of objects that refer only to each other. A class with a `__del__` method shows
it: setting the only variable to `None` frees the object before the next line runs. The
JavaScript, Dart, and JVM collectors instead run from time to time and look for
everything that has become unreachable, so you cannot say exactly when an object goes.
None of these languages lets you free an object yourself. Chapter 10 covers what to do
for files and other resources that must be closed on time.

### Go: values, pointers, and a collector

Go keeps C's value semantics and adds a garbage collector.

**Go**

```go
package main

import (
	"fmt"
	"slices"
)

type Point struct {
	X, Y int
}

// Returning the address of a local is fine in Go. The compiler sees that n
// outlives the call and puts it on the heap.
func newCounter() *int {
	n := 0
	return &n
}

func main() {
	// Arrays are values: assignment copies every item.
	a := [3]int{1, 2, 3}
	b := a
	b[0] = 100
	fmt.Printf("b := a copies an array: a = %v, b = %v\n", a, b)

	// A slice is a small header (pointer, length, capacity) over an array.
	// Copying the header shares the array.
	s := []int{1, 2, 3}
	t := s
	t[0] = 100
	fmt.Printf("t := s shares a slice's items: s = %v\n", s)

	// append writes into the shared array when there is room.
	base := make([]int, 3, 4) // length 3, room for 4
	x := append(base, 1)
	y := append(base, 2) // the same spare slot as x
	fmt.Printf("two appends into one spare slot: x = %v, y = %v\n", x, y)

	// Structs are values too.
	p := Point{1, 2}
	q := p
	q.X = 10
	fmt.Printf("q := p copies a struct: p = %v, q = %v\n", p, q)

	// A pointer shares on purpose. r.X is short for (*r).X.
	r := &p
	r.X = 10
	fmt.Printf("r := &p shares it: after r.X = 10, p = %v\n", p)

	// slices.Clone copies the outer slice only, so the rows are shared.
	grid := [][]int{{1, 2}, {3, 4}}
	shallow := slices.Clone(grid)
	shallow[0][0] = 9
	fmt.Printf("slices.Clone(grid) is shallow: grid = %v\n", grid)

	c := newCounter()
	*c++
	fmt.Printf("a pointer to a local outlives its function: *c = %d\n", *c)
}
```

*Go:*

```text
b := a copies an array: a = [1 2 3], b = [100 2 3]
t := s shares a slice's items: s = [100 2 3]
two appends into one spare slot: x = [0 0 0 2], y = [0 0 0 2]
q := p copies a struct: p = {1 2}, q = {10 2}
r := &p shares it: after r.X = 10, p = {10 2}
slices.Clone(grid) is shallow: grid = [[9 2] [3 4]]
a pointer to a local outlives its function: *c = 1
```

**Arrays and structs are values.** `b := a` copies a `[3]int`, and `q := p` copies a
struct, just as in C. A pointer shares on purpose: `r := &p` and `r.X = 10` change `p`.
Go has no `->`, because `r.X` follows the pointer automatically, and no pointer
arithmetic: `p + 1` is `invalid operation: p + 1 (mismatched types *int and untyped
int)`.

**A slice is a header that shares an array.** Like a C++ vector's header, a slice is a
pointer, a length, and a capacity: 24 bytes. `t := s` copies the header, so both share
the items. `append` writes into the shared array when it has room, and allocates a new
array when it does not. So the two `append`s to `base`, which has length 3 and room for
4, both write into the same spare slot, and `x` and `y` end up equal. Whether two
slices share depends on capacity, which the code does not show, so the safe habit is
`s = append(s, v)`, always assigning back to the same slice. `slices.Clone` (Go 1.21)
copies one level, so for a slice of slices it shares the rows, as in the other
languages.

**The compiler picks the stack or the heap.** Returning `&n` from `newCounter` is a bug
in C and an error in Rust, but correct in Go. The compiler's **escape analysis** sees
that `n` outlives the call and puts it on the heap. `go build -gcflags=-m` shows the
decisions: `moved to heap: n` for this function, and `new(int) does not escape` for a
`new(int)` that never leaves its function, which stays on the stack. In Go, `new` and
`&` say nothing about where memory lives.

---

## 9.9 Why each language looks the way it does

- **C** was written in the early 1970s to write Unix, and it maps closely onto the
  machine. A pointer is an address, an array is a run of bytes, and a struct is its
  fields laid end to end. C does exactly what you write: it copies the bytes you name,
  frees what you free, and checks nothing, which is why it is fast and why its bugs
  are memory bugs.
- **C++** kept C's model and added references, chiefly so that overloaded operators
  such as `a[i] = x` and `a + b` could work on objects without copying them or taking
  addresses. It gave its library containers value semantics, so a `std::vector`
  behaves like an `int`: assignment copies and destruction frees. Pointers into those
  containers can still dangle, and the compiler does not check them.
- **Rust** keeps C++'s value semantics and adds one rule the compiler checks: many
  readers or one writer, and no reference outliving its value. That rule removes
  dangling references, iterator invalidation, and aliasing bugs from safe code, with
  no garbage collector. It costs the moves and borrow errors that chapter 10 explains.
- **Go** keeps C's control over memory layout and removes its commonest crashes. Structs
  and arrays are values, pointers exist but cannot do arithmetic, escape analysis makes
  `&local` safe, and a garbage collector frees the heap. Slices are the one place where
  values and sharing meet, and they surprise most newcomers at least once.
- **Python** treats everything as an object reached through a reference, including
  numbers, functions, and classes. That makes the language simple to describe, at the
  cost of every value being a heap object. CPython's reference counting frees most
  objects right away, which many programs rely on even though the language does not
  promise it.
- **JavaScript** splits values into primitives, which are copied, and objects, which
  are shared, and it has never exposed addresses. Without a deep copy for most of its
  history, it relied on `JSON.parse(JSON.stringify(x))` until `structuredClone`
  arrived in Node 17 and the major browsers in 2021 and 2022.
- **TypeScript** cannot change JavaScript's memory model, only describe it. `readonly`
  and `as const` let the compiler stop accidental changes to shared objects, and they
  vanish when the types are erased.
- **Dart** follows the same model as JavaScript and Java: everything is an object,
  reached through a reference and freed by a garbage collector. Its generational
  collector handles many short-lived objects cheaply, which suits Flutter, where
  widgets are rebuilt all the time.
- **Kotlin** inherits the JVM's model of primitives and references and hides the
  difference. `Int` is one type in the source and becomes a plain `int` or a boxed
  `Integer` as needed. Its split between read-only `List` and `MutableList` is a
  compromise with Java, whose `java.util.List` has methods to change any list.

---

## 9.10 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | Returning the address of a local | A warning; GCC makes the function return `NULL` |
| C | `sizeof` on an array parameter | The size of a pointer; `-Wsizeof-array-argument` warns |
| C | Writing to a string literal through a `char *` | Undefined behavior; on Linux, `Segmentation fault` |
| C | Assigning one array to another | `error: assignment to expression with array type` |
| C | Copying a struct that holds a pointer, then freeing both copies | A double free: `free(): double free detected in tcache 2` |
| C, C++ | The XOR or add-and-subtract swap on one variable | The value becomes 0 |
| C, C++ | A `const` parameter that aliases one the function writes | The "constant" changes partway through |
| C++ | Keeping a reference or pointer into a vector across `push_back` | Undefined behavior; AddressSanitizer reports `heap-use-after-free` |
| C++ | `for (auto x : v)` to change the items | Changes copies; use `auto &` |
| C++ | Passing a `std::vector` or `std::string` by value to read it | Copies every item; use `const &` |
| Rust | `mem::swap(&mut v[i], &mut v[j])` | E0499; use `v.swap(i, j)` or `split_at_mut` |
| Rust | Holding `&v[0]` across `v.push(...)` | E0502 |
| Rust | Using a value after `let b = a;` | E0382: `a` was moved, not copied |
| Go | Two `append`s to one slice that has spare capacity | Both write the same slot |
| Go | `copy` or `slices.Clone` on a slice of slices | The rows are shared |
| Python | `[[0] * w] * h` | One row, `h` times |
| Python | A list as a default argument, `def f(items=[])` | One list shared by every call |
| Python | `is` to compare numbers or strings | Depends on caching; use `==` |
| JavaScript, TypeScript | `Array(h).fill([])` | One array in every slot |
| JavaScript, TypeScript | `const xs = [...]` thought to freeze the array | It only stops `xs` from being reassigned |
| JavaScript | `Object.freeze` thought to freeze nested objects | It is shallow |
| Dart | `List.filled(h, [])` | One list in every slot |
| Dart | `==` on two lists with equal items | `false`: it compares identity |
| Kotlin | `copy()` on a data class with a `MutableList` field | The copy shares the list |
| Kotlin | A `List<T>` treated as unchangeable | Its owner may still change it |
| All with references | `b = a` to make a copy | Both names share one object |

---

## 9.11 Idiom check

**C: say who owns each pointer, and pass lengths with arrays.** Pass `const T *` for
input and `T *` for output, and write `size_t len` next to every array parameter. Keep
fixed-size data in arrays inside structs when copies should be deep, and write a copy
function for structs that hold pointers. Set a pointer to `NULL` after freeing
it, and build with `-fsanitize=address` while developing.

**C++: values by default, references for reach.** Let `std::vector`, `std::string`, and
`std::array` own memory, and pass them as `const T &` to read or `T &` to change. Use
`auto &` or `const auto &` in range `for` loops unless you want copies. Never keep a
pointer, reference, or iterator into a container across an operation that can grow it.

**Rust: borrow, and clone on purpose.** Take `&[T]` and `&str` parameters rather than
`&Vec<T>` and `&String`. Reach for `iter_mut`, `swap`, and `split_at_mut` when two
places in one collection must change, and copy a value out first when it must be read
while the collection changes. Write `.clone()` only where a second copy is really
needed, and derive `Copy` only for small plain types.

**Go: values for small things, pointers to share or change.** Use pointer receivers and
`*T` parameters when a function must change a struct or the struct is large. Always
write `s = append(s, v)`, and use `slices.Clone` or `copy` before handing a slice to
code that may change it. Trust escape analysis rather than avoiding `&`.

**Python: copy at the boundary.** Build nested lists with a comprehension, copy a list
before changing one you were given (`items = list(items)`), and use `None` as the
default for a list argument. Prefer tuples and frozen dataclasses for data that should
not change, and compare with `==`, keeping `is` for `None`.

**JavaScript and TypeScript: spread, `structuredClone`, and `readonly`.** Use
`Array.from({ length: h }, () => ...)` for grids, `[...xs]` and `{ ...obj }` for
shallow copies, and `structuredClone` for deep ones. In TypeScript, mark parameters
`readonly T[]` when a function should only read them.

**Dart: `List.generate`, `List.of`, and unmodifiable views.** Build grids with
`List.generate`, copy with `List.of` or a collection `for`, and return
`List.unmodifiable` or `UnmodifiableListView` from a class that keeps its own list.

**Kotlin: expose `List`, keep `MutableList` private.** Build nested lists with
`MutableList(h) { MutableList(w) { 0 } }`, make defensive copies with `toList()`, and
use `val` with immutable fields in data classes so a shallow `copy()` is safe.

---

## 9.12 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Swap](exercises/01-swap/) | A swap function in each language's style: pointers, references, or a list and two indexes; swapping a value with itself |
| 2 | [Snapshots](exercises/02-snapshots/) | Copying a grid deeply, and the three aliasing bugs waiting in it |
| 3 | [A growable array in C](exercises/03-growable-array/) | `malloc`, `realloc`, and `free`, capacity doubling, strict number parsing |

```sh
python3 tools/check.py 09                     # your answers, every language
python3 tools/check.py 09 --lang c,rust       # some languages
python3 tools/check.py 09 --solutions         # the reference answers
```

Exercise 3 is C only, so the checker skips it for the other languages.

**Stretch:** make exercise 2's snapshots cheap. Instead of copying the whole grid on
every `save`, keep rows that cannot change, such as Python strings or tuples, Kotlin
`String`s, or Rust `Rc<str>`, so that a snapshot copies only the list of row
references. Painting a cell then replaces one row with a new one. Which aliasing bugs
are still possible, and why do the others go away?

---

Previous: [Chapter 8: Structs, enums, and null](../08-structs-enums-and-null/) ·
Next: Chapter 10: Ownership and resource management (coming soon)
