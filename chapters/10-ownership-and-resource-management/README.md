# Chapter 10: Ownership and resource management

Chapter 9 showed where values live. This chapter asks who gives them back. A program
borrows memory, files, connections, and locks from the system, and each of them has to
be returned exactly once, after the last time it is used. C leaves that to you. C++ and
Rust tie it to scopes, so the compiler writes the cleanup calls for you, and Rust adds
rules that stop you from using something after it is gone. Go, Python, JavaScript,
TypeScript, Dart, and Kotlin hand memory to a garbage collector, but they still need you
to close everything else, and they all give you a block for it. C, C++, and Rust get
most of the chapter. The other six share section 10.9.

**Languages:** C, C++, and Rust in full; Go, Python, JavaScript, TypeScript, Dart, and
Kotlin together in section 10.9 · **Time:** about 3 hours

| You will learn | Why it matters later |
| --- | --- |
| What owning a resource means, and the three ways a language can give it back | Every program that opens a file or a connection |
| Cleanup that runs on every path: `goto` labels, destructors, `Drop`, `defer`, `with`, `finally`, and `use` | Error handling (chapter 11), files (chapter 20), databases (chapter 33) |
| Moving ownership, sharing it with reference counts, and the cycles that leak | Linked lists and trees (chapters 25 and 26) |
| How Rust's borrow checker uses lifetimes | Iterators (chapter 14), concurrency (chapter 16) |
| Finding leaks and use-after-free bugs with AddressSanitizer and Valgrind | Debugging and profiling (chapter 19), FFI (chapter 22) |

---

## 10.1 The concept: every resource has an owner

A **resource** is anything a program gets from the system and must give back: a block of
heap memory, an open file, a network connection, a lock, a database transaction. The
system has a limited supply of each. The resource's **owner** is the one part of the
program responsible for giving it back, exactly once, after its last use. There are
three ways to get that wrong:

- A **leak** never gives it back. Leaked memory is wasted until the program exits. A
  leaked file stays open, and once the process reaches its limit, every later open fails
  with `Too many open files`.
- A **double free** gives it back twice. The second `free` can corrupt the allocator's
  records, and a second close can shut a file that another part of the program has since
  opened.
- A **use after free** keeps using it after giving it back. The memory may already hold
  something else, so the program reads or writes someone else's data.

Languages answer "who gives it back?" in one of three ways:

- **You do, in C.** Every `malloc` needs a `free` and every `fopen` an `fclose`, on
  every path through the code, including the paths that fail halfway. Nothing checks.
- **The scope does, in C++ and Rust.** The owner is a variable. When it goes out of
  scope, whether the block ends, the function returns, or an exception or panic passes
  through, the compiler calls the owner's **destructor**, which gives the resource back.
  C++ calls the pattern **RAII**, *resource acquisition is initialization*: acquire in a
  constructor, release in the destructor. Rust builds it into the language and adds two
  rules that the compiler checks: every value has exactly one owner, and a reference
  must not outlive the value it refers to.
- **The garbage collector does, for memory only, in Go, Python, JavaScript, TypeScript,
  Dart, and Kotlin.** Memory is freed some time after nothing refers to it. A file or a
  lock still has to be closed when you are done with it, so these languages give you a
  block that runs however it ends: `defer`, `with`, `finally`, or `use`.

```text
 C                        C++ and Rust                 garbage collected
 p = malloc(...);         {                            f = open(...)
 if (error)                   File f("data");          try:
     goto cleanup;            ...                          ...
 ...                      }   // the destructor        finally:
 cleanup:                     // closes f here             f.close()
 free(p);                                              # the collector frees memory later
```

Ownership can also change hands. **Moving** gives a resource to a new owner and leaves
the old one with nothing to free. **Sharing** lets several owners hold one resource and
counts them, so that the last owner to let go frees it. A pointer or reference that does
not own is a **borrow**: it may use the resource, but it must not outlive the owner.

---

## 10.2 What each language gives you

| Language | Heap memory is freed by | Files, sockets, and locks are closed by | Moving ownership | Shared ownership | A cycle of owners |
| --- | --- | --- | --- | --- | --- |
| C | You, with `free` | You, with `fclose` and friends, often through `goto` cleanup labels | Copy the pointer and set the old one to `NULL`, by convention | Count by hand | Leaks unless you break it |
| C++ | Destructors: containers, `std::unique_ptr`, `std::shared_ptr` | Destructors: `std::fstream`, `std::lock_guard` | `std::move`; the source is left valid but unspecified | `std::shared_ptr`, `std::weak_ptr` | Leaks; break it with `weak_ptr` |
| Rust | `Drop`, when the owner goes out of scope | `Drop`: `File`, `MutexGuard` | The default for assignment and passing; the compiler stops later use | `Rc`, `Arc`, `Weak` | Leaks; break it with `Weak` |
| Go | Garbage collector | `defer f.Close()` | Not needed: values are copied, pointers shared | Any pointer | Collected |
| Python | Reference counting, plus a cycle collector | `with` | Not needed | Any reference | Collected by the cycle collector |
| JavaScript | Garbage collector | `try`/`finally`; `using` in newer engines | Not needed | Any reference | Collected |
| TypeScript | As JavaScript | As JavaScript; `using` type-checks | Not needed | Any reference | Collected |
| Dart | Garbage collector | `try`/`finally` | Not needed | Any reference | Collected |
| Kotlin | The JVM's garbage collector | `use` on an `AutoCloseable` | Not needed | Any reference | Collected |

Read the table by its second and third columns. For heap memory, the languages split
three ways: C by hand, C++ and Rust by scope, and the rest by a collector. For files,
sockets, and locks, the split is only two ways. C++ and Rust close them the same way
they free memory, and every other language, collected or not, needs you to say where the
cleanup goes.

---

## 10.3 Manual ownership in C

In C, ownership is something the program says, not something the compiler knows. This
example says it in comments, and pairs every function that allocates with one that
frees:

**C**

```c
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C has no idea who owns a block of memory. The program says it in names and
// comments, and every function that allocates is paired with one that frees.

typedef struct {
    char *name;  // owned: player_free frees it
    int *scores; // owned
    size_t count;
} Player;

// Returns a copy of text that the caller owns, or NULL if memory ran out.
// (strdup does this too, but it is not in C17.)
char *copy_string(const char *text) {
    size_t size = strlen(text) + 1;
    char *copy = malloc(size);
    if (copy != NULL) {
        memcpy(copy, text, size);
    }
    return copy;
}

// Returns a new player that the caller owns and must give to player_free, or
// NULL if memory ran out. If a later step fails, the earlier ones are undone.
Player *player_new(const char *name, size_t count) {
    Player *p = malloc(sizeof *p);
    if (p == NULL) {
        return NULL;
    }
    p->name = copy_string(name);
    if (p->name == NULL) {
        free(p);
        return NULL;
    }
    p->scores = calloc(count, sizeof *p->scores);
    if (p->scores == NULL) {
        free(p->name);
        free(p);
        return NULL;
    }
    p->count = count;
    return p;
}

// Frees p and everything it owns, in the reverse order of player_new.
// Like free, it does nothing when p is NULL.
void player_free(Player *p) {
    if (p == NULL) {
        return;
    }
    free(p->scores);
    free(p->name);
    free(p);
}

// Borrows p: reads it and keeps nothing.
int player_total(const Player *p) {
    int total = 0;
    for (size_t i = 0; i < p->count; i++) {
        total += p->scores[i];
    }
    return total;
}

// Takes ownership of name, which must come from malloc. The player frees it.
void player_rename(Player *p, char *name) {
    free(p->name);
    p->name = name;
}

// Returns a new string that the caller owns and must free.
char *greeting(const Player *p) {
    const char *hello = "Hello, ";
    size_t size = strlen(hello) + strlen(p->name) + 2;
    char *text = malloc(size);
    if (text != NULL) {
        snprintf(text, size, "%s%s!", hello, p->name);
    }
    return text;
}

int main(void) {
    Player *ada = player_new("Ada", 3);
    if (ada == NULL) {
        fputs("out of memory\n", stderr);
        return 1;
    }
    ada->scores[0] = 90;
    ada->scores[1] = 85;
    ada->scores[2] = 80;
    printf("%s has %zu scores, total %d\n", ada->name, ada->count,
           player_total(ada));

    // After the call the player owns the new name, so this function must not
    // free it or use it. Setting the pointer to NULL makes that hard to forget.
    char *name = copy_string("Ada Lovelace");
    if (name == NULL) {
        player_free(ada);
        return 1;
    }
    player_rename(ada, name);
    name = NULL;
    printf("renamed to %s\n", ada->name);

    // The caller owns what greeting returns.
    char *hello = greeting(ada);
    if (hello != NULL) {
        printf("%s\n", hello);
        free(hello);
    }

    // A pointer into the player is borrowed: it is good until the player is
    // freed, and the compiler does not check that.
    const char *borrowed = ada->name;
    printf("borrowed name: %s\n", borrowed);

    player_free(ada);
    ada = NULL;
    // borrowed now dangles. Reading it is undefined behavior, so don't.

    player_free(ada); // ada is NULL now, so this does nothing
    printf("the second player_free(ada) freed nothing, because ada is NULL\n");
    return 0;
}
```

*C:*

```text
Ada has 3 scores, total 255
renamed to Ada Lovelace
Hello, Ada Lovelace!
borrowed name: Ada Lovelace
the second player_free(ada) freed nothing, because ada is NULL
```

**Ownership is a convention.** Nothing in the type `Player *` says whether you must free
it. The comments do: `player_new` returns a player that the caller owns, `player_total`
borrows one and keeps nothing, and `player_rename` takes ownership of the string it is
given. C libraries spell the same thing with names, such as `_new` and `_free`, or
`_create` and `_destroy`, and with a line in the documentation such as "the caller must
free the result", which is what POSIX says about `strdup` and `getline`.

**Undo partial work, in the reverse order.** `player_new` makes three allocations, and
if the third fails it frees the first two before returning `NULL`. `player_free` frees
in the reverse order of `player_new`: the strings first, then the struct that pointed to
them. Free the struct first and you can no longer reach its strings.

**`free(NULL)` does nothing,** so a function that frees should accept `NULL` too, as
`player_free` does. Setting a pointer to `NULL` after freeing it, as `main` does with
`ada`, turns a second free into a harmless no-op, and on the usual systems turns a later
use into an immediate crash rather than a quiet read of freed memory.

**A borrowed pointer is good until the owner frees it, and nothing checks.** `borrowed`
points into the player's name, so it dangles the moment `player_free(ada)` runs. Reading
it then is undefined behavior: it might print the old name, print garbage, or crash. GCC
12 and newer catch the simplest case at compile time, when the `free` and the use are in
the same function: `warning: pointer 'p' used after 'free' [-Wuse-after-free]`, which
`-Wall` turns on. They miss most cases where the `free` and the use are in different
functions. Section 10.10 shows the tools that catch those when the program runs.

---

## 10.4 Cleanup on every path

Freeing at the end is easy. Freeing on every early exit is where leaks come from. Here
is one program in all nine languages: a job opens two pretend files, copies one into the
other, and can fail in two places. The files print when they open and close. The C, C++,
and Rust versions are below, and the other six are in section 10.9.

**C**

```c
#include <stdbool.h>
#include <stdio.h>

// A pretend file that prints when it opens and closes.
typedef struct {
    const char *name;
} File;

bool file_open(File *f, const char *name, bool fails) {
    if (fails) {
        return false;
    }
    f->name = name;
    printf("  open %s\n", name);
    return true;
}

void file_close(File *f) {
    printf("  close %s\n", f->name);
}

// Returns NULL on success, or a message saying what failed. Every exit after
// the first open goes through the labels at the bottom, which close what was
// opened in the reverse order.
const char *run_job(bool output_fails, bool copy_fails) {
    const char *error = NULL;
    File input;
    File output;
    if (!file_open(&input, "input", false)) {
        return "cannot open input";
    }
    if (!file_open(&output, "output", output_fails)) {
        error = "cannot open output";
        goto close_input;
    }
    if (copy_fails) {
        error = "disk full";
        goto close_output;
    }
    printf("  copy input to output\n");

close_output:
    file_close(&output);
close_input:
    file_close(&input);
    return error;
}

int main(void) {
    struct {
        const char *title;
        bool output_fails;
        bool copy_fails;
    } jobs[] = {
        {"everything works", false, false},
        {"opening output fails", true, false},
        {"copying fails", false, true},
    };
    for (int i = 0; i < 3; i++) {
        printf("job %d: %s\n", i + 1, jobs[i].title);
        const char *error = run_job(jobs[i].output_fails, jobs[i].copy_fails);
        if (error == NULL) {
            printf("  job %d done\n", i + 1);
        } else {
            printf("  job %d failed: %s\n", i + 1, error);
        }
    }
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

// A pretend file that opens in its constructor and closes in its destructor.
// That pairing is called RAII: resource acquisition is initialization.
class File {
  public:
    File(std::string name, bool fails) : name_(std::move(name)) {
        if (fails) {
            throw std::runtime_error("cannot open " + name_);
        }
        std::cout << "  open " << name_ << '\n';
    }
    ~File() { std::cout << "  close " << name_ << '\n'; }

    // Two File objects closing one file would close it twice, so no copies.
    File(const File &) = delete;
    File &operator=(const File &) = delete;

  private:
    std::string name_;
};

// No cleanup code at all. When the function ends, by returning or by an
// exception, the files it opened are destroyed in the reverse order. A
// constructor that throws never made an object, so there is nothing to close.
void run_job(bool output_fails, bool copy_fails) {
    File input("input", false);
    File output("output", output_fails);
    if (copy_fails) {
        throw std::runtime_error("disk full");
    }
    std::cout << "  copy input to output\n";
}

int main() {
    struct Job {
        std::string title;
        bool output_fails;
        bool copy_fails;
    };
    Job jobs[] = {
        {"everything works", false, false},
        {"opening output fails", true, false},
        {"copying fails", false, true},
    };
    int number = 0;
    for (const Job &job : jobs) {
        number++;
        std::cout << "job " << number << ": " << job.title << '\n';
        try {
            run_job(job.output_fails, job.copy_fails);
            std::cout << "  job " << number << " done\n";
        } catch (const std::exception &e) {
            std::cout << "  job " << number << " failed: " << e.what() << '\n';
        }
    }
}
```

**Rust**

```rust
// A pretend file that closes when it is dropped.
struct File {
    name: &'static str,
}

impl File {
    fn open(name: &'static str, fails: bool) -> Result<File, String> {
        if fails {
            return Err(format!("cannot open {name}"));
        }
        println!("  open {name}");
        Ok(File { name })
    }
}

impl Drop for File {
    fn drop(&mut self) {
        println!("  close {}", self.name);
    }
}

// No cleanup code at all. When the function returns, whether at the end or
// early through `?`, its owned values are dropped in the reverse order.
fn run_job(output_fails: bool, copy_fails: bool) -> Result<(), String> {
    let _input = File::open("input", false)?;
    let _output = File::open("output", output_fails)?;
    if copy_fails {
        return Err("disk full".to_string());
    }
    println!("  copy input to output");
    Ok(())
}

fn main() {
    let jobs = [
        ("everything works", false, false),
        ("opening output fails", true, false),
        ("copying fails", false, true),
    ];
    for (i, (title, output_fails, copy_fails)) in jobs.into_iter().enumerate() {
        let number = i + 1;
        println!("job {number}: {title}");
        match run_job(output_fails, copy_fails) {
            Ok(()) => println!("  job {number} done"),
            Err(message) => println!("  job {number} failed: {message}"),
        }
    }
}
```

All nine print the same lines. Each file that opened is closed, last opened first, and
the job's result comes after the closes:

*C, C++, Rust, Go, Python, JavaScript, TypeScript, Dart and Kotlin:*

```text
job 1: everything works
  open input
  open output
  copy input to output
  close output
  close input
  job 1 done
job 2: opening output fails
  open input
  close input
  job 2 failed: cannot open output
job 3: copying fails
  open input
  open output
  close output
  close input
  job 3 failed: disk full
```

**C: one exit, with a label for each thing to undo.** Every way out of `run_job` after
the first open goes through the labels at the bottom. A failure jumps to the label for
what is open so far: when `output` fails to open there is nothing to close but `input`,
so it jumps past `close_output`. The labels come in the reverse order of the opens, so
falling through them closes everything. This `goto` pattern is the standard C idiom,
used throughout the Linux kernel. GCC and Clang also offer a non-standard attribute,
`__attribute__((cleanup(free_ptr))) char *buffer`, that calls `free_ptr(&buffer)` when
`buffer` goes out of scope, but code that must build with any C compiler can't use it.

**C++: destructors, and no cleanup code.** `run_job` has none. When it ends, by
returning or by an exception passing through, `output` and then `input` are destroyed,
and their destructors close them. This is called **stack unwinding**. A constructor that
throws never made an object, so its destructor does not run, which is right: the file
never opened. `File` deletes its copy constructor because two `File` objects for one
file would close it twice. A class that closes a `FILE *` in its destructor and is
copied by accident crashes on the second close with
`free(): double free detected in tcache 2`. The standard library works the same way
throughout: `std::fstream` closes its file, `std::lock_guard` unlocks its mutex, and
`std::vector` frees its items.

**Rust: `Drop`, and no cleanup code either.** `?` returns early when an open fails, and
every value the function owns is dropped on the way out, last declared first. A panic
unwinds the stack and runs the same drops: the panic message prints first, then the
drops run. Rust's `File` closes on drop and a `MutexGuard` unlocks on drop, so the
pattern is the same for every resource. The one surprise is in the names:
`let _input = ...` keeps the file open until the end of the function, but `let _ = ...`
would drop it at once. Section 10.5 shows why.

---

## 10.5 Destructors and drop order

C++ and Rust both run destructors at the end of a scope, in a fixed order you can rely
on:

**C++**

```cpp
#include <iostream>
#include <optional>
#include <string>
#include <utility>

// Prints when it is made and when it is destroyed.
class Noisy {
  public:
    explicit Noisy(std::string name) : name_(std::move(name)) {
        std::cout << "  make " << name_ << '\n';
    }
    ~Noisy() { std::cout << "  drop " << name_ << '\n'; }
    Noisy(const Noisy &) = delete;
    Noisy &operator=(const Noisy &) = delete;

  private:
    std::string name_;
};

// The destructor's body runs first. Then the members are destroyed, in the
// reverse of the order they are declared in.
struct Pair {
    Noisy first{"pair.first"};
    Noisy second{"pair.second"};
    ~Pair() { std::cout << "  drop pair\n"; }
};

int main() {
    std::cout << "locals go in the reverse order:\n";
    {
        Noisy a("a");
        Noisy b("b");
        Noisy c("c");
    }

    std::cout << "an inner scope ends first:\n";
    {
        Noisy outer("outer");
        {
            Noisy inner("inner");
        }
        std::cout << "  inner is gone, outer is not\n";
    }

    std::cout << "a struct, then its members:\n";
    {
        Pair pair;
    }

    std::cout << "a temporary lasts until the end of its statement:\n";
    Noisy("temporary");
    std::cout << "  the temporary is gone\n";

    std::cout << "ending a lifetime early:\n";
    {
        std::optional<Noisy> early;
        early.emplace("early");
        early.reset();
        std::cout << "  early is gone before its scope ends\n";
    }
}
```

**Rust**

```rust
// Prints when it is made and when it is dropped.
struct Noisy(&'static str);

fn make(name: &'static str) -> Noisy {
    println!("  make {name}");
    Noisy(name)
}

impl Drop for Noisy {
    fn drop(&mut self) {
        println!("  drop {}", self.0);
    }
}

// drop runs first. Then the fields are dropped, in the order they are
// declared in: the opposite of C++. (The underscores tell the compiler that
// nothing reads the fields.)
struct Pair {
    _first: Noisy,
    _second: Noisy,
}

impl Drop for Pair {
    fn drop(&mut self) {
        println!("  drop pair");
    }
}

fn main() {
    println!("locals go in the reverse order:");
    {
        let _a = make("a");
        let _b = make("b");
        let _c = make("c");
    }

    println!("an inner scope ends first:");
    {
        let _outer = make("outer");
        {
            let _inner = make("inner");
        }
        println!("  inner is gone, outer is not");
    }

    println!("a struct, then its fields:");
    {
        let _pair = Pair {
            _first: make("pair.first"),
            _second: make("pair.second"),
        };
    }

    println!("a temporary lasts until the end of its statement:");
    make("temporary");
    println!("  the temporary is gone");

    // `let _` binds nothing, so the value is a temporary too. `let _name`
    // binds a variable, which lives to the end of the scope.
    println!("let _ = drops at once, let _name = does not:");
    {
        let _ = make("unnamed");
        let _named = make("named");
        println!("  the scope ends");
    }

    println!("ending a lifetime early:");
    {
        let early = make("early");
        drop(early);
        println!("  early is gone before its scope ends");
    }
}
```

*C++:*

```text
locals go in the reverse order:
  make a
  make b
  make c
  drop c
  drop b
  drop a
an inner scope ends first:
  make outer
  make inner
  drop inner
  inner is gone, outer is not
  drop outer
a struct, then its members:
  make pair.first
  make pair.second
  drop pair
  drop pair.second
  drop pair.first
a temporary lasts until the end of its statement:
  make temporary
  drop temporary
  the temporary is gone
ending a lifetime early:
  make early
  drop early
  early is gone before its scope ends
```

*Rust:*

```text
locals go in the reverse order:
  make a
  make b
  make c
  drop c
  drop b
  drop a
an inner scope ends first:
  make outer
  make inner
  drop inner
  inner is gone, outer is not
  drop outer
a struct, then its fields:
  make pair.first
  make pair.second
  drop pair
  drop pair.first
  drop pair.second
a temporary lasts until the end of its statement:
  make temporary
  drop temporary
  the temporary is gone
let _ = drops at once, let _name = does not:
  make unnamed
  drop unnamed
  make named
  the scope ends
  drop named
ending a lifetime early:
  make early
  drop early
  early is gone before its scope ends
```

**Locals go in the reverse order they were made.** `c` was made last, so it is destroyed
first. A later variable may hold a reference to an earlier one, such as a file and the
buffer that writes to it, so destroying the newest first keeps every reference good
until it is gone. An inner block ends its own variables before the code after it runs.

**Fields are where C++ and Rust differ.** Both run the struct's own destructor body
first. Then C++ destroys the members in the reverse order of their declarations, which
mirrors the order it built them in, while Rust drops the fields in declaration order.
The other rules match, and neither language lets the order vary.

**Temporaries go at the end of their statement.** `Noisy("temporary");` and
`make("temporary");` build a value nobody names, and it is destroyed before the next
line runs. Rust's `let _ = make("unnamed")` is the same: `_` is not a variable, so
nothing keeps the value, and it is dropped at once. `let _named` is a real variable, and
the underscore only stops the compiler from warning that it is unused, so it lives to
the end of the scope. The difference matters most with locks, where
`let _ = mutex.lock()` would unlock at once. The compiler refuses that one,
`error: non-binding let on a synchronization lock`, and suggests
`let _unused = mutex.lock()`, which holds the lock to the end of the block.

**Ending a lifetime early.** In Rust, `drop(early)` moves the value into a function that
does nothing with it, so it is dropped there. Calling the destructor yourself is
refused: `early.drop()` is `error[E0040]: explicit use of destructor method`. C++ has no
general way to end a variable early, but types that might be empty have a method for it:
`std::optional::reset` and `std::unique_ptr::reset` destroy what they hold. An inner
block works in both languages.

**A destructor must not fail.** C++ destructors are `noexcept` unless you say otherwise,
so a destructor that throws ends the program. GCC warns,
`'throw' will always call 'terminate' [-Wterminate]`, and at run time it prints
`terminate called after throwing an instance of 'std::runtime_error'` and aborts. Rust's
`drop` returns nothing, so it cannot report an error either. A type whose cleanup can
fail therefore offers a method you call first and check: `std::ofstream::close`, and in
Rust `BufWriter::flush`, whose documentation warns that errors while dropping are
ignored.

**A destructor changes what copying means.** In C++, a class that releases something in
its destructor needs to decide what a copy does, which is the **rule of three**
(destructor, copy constructor, copy assignment) and, with moves, the **rule of five**.
The best answer is usually the **rule of zero**: hold every resource in a member that
already manages it, such as `std::string`, `std::vector`, or `std::unique_ptr`, and
write no destructor at all. Rust settles the question in the compiler: a type with
`Drop` cannot be `Copy`,
`error[E0184]: the trait 'Copy' cannot be implemented for this type; the type has a destructor`,
so the only way to duplicate it is a `clone` you write or derive.

---

## 10.6 Moving ownership

Copying a value that owns a resource means copying the resource, which is slow for a big
vector and impossible for an open file. Moving hands the resource itself to a new owner:

**C**

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C cannot move anything. "Moving" ownership means copying a pointer and
// agreeing that the old place no longer owns the block. Setting the old
// pointer to NULL turns that agreement into something the code can check.

typedef struct {
    char *text; // owned, or NULL
} Message;

const char *show(const Message *m) {
    return m->text == NULL ? "(nothing)" : m->text;
}

// Moves the text out of from and into to, freeing whatever to held before.
void message_move(Message *to, Message *from) {
    if (to == from) {
        return;
    }
    free(to->text);
    to->text = from->text;
    from->text = NULL;
}

int main(void) {
    Message a = {malloc(6)};
    Message b = {NULL};
    if (a.text == NULL) {
        return 1;
    }
    strcpy(a.text, "hello");

    // b = a would copy the pointer and leave two owners, and freeing both
    // would free the block twice.
    message_move(&b, &a);
    printf("after message_move(&b, &a): a = %s, b = %s\n", show(&a), show(&b));

    // Each Message frees what it holds. a holds NULL, and free(NULL) does
    // nothing, so the block is freed exactly once.
    free(a.text);
    free(b.text);
    printf("freed both, and the block was freed once\n");
    return 0;
}
```

**C++**

```cpp
#include <cstddef>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

// Says which constructor made it, and how many ints it had to copy.
class Loud {
  public:
    explicit Loud(std::vector<int> data) : data_(std::move(data)) {}
    Loud(const Loud &other) : data_(other.data_) {
        std::cout << "  copied " << data_.size() << " ints\n";
    }
    Loud(Loud &&other) noexcept : data_(std::move(other.data_)) {
        std::cout << "  moved, copied 0 ints\n";
    }
    Loud &operator=(const Loud &) = delete;
    Loud &operator=(Loud &&) = delete;
    std::size_t size() const { return data_.size(); }

  private:
    std::vector<int> data_;
};

// Takes its argument by value, so the caller decides: copy or move.
std::size_t keep(Loud loud) { return loud.size(); }

Loud make_loud() {
    Loud made({1, 2, 3});
    // Compilers build made in the caller's place, so this usually neither
    // copies nor moves. Where they can't, return moves.
    return made;
}

int main() {
    std::vector<int> a{1, 2, 3};
    std::vector<int> b = a;            // a copy
    std::vector<int> c = std::move(a); // a move: c takes a's heap block
    std::cout << "copy then move: a has " << a.size() << ", b has "
              << b.size() << ", c has " << c.size() << '\n';

    // A unique_ptr can only be moved. `auto q = p;` does not compile.
    auto p = std::make_unique<int>(7);
    auto q = std::move(p);
    std::cout << "after the move, p is " << (p ? "full" : "empty")
              << " and *q = " << *q << '\n';

    std::cout << "passing by value:\n";
    Loud big({1, 2, 3, 4, 5});
    keep(big);
    keep(std::move(big));
    // big is still here, but moved-from: valid, with an unspecified value.

    std::cout << "returning a local:\n";
    Loud made = make_loud();
    std::cout << "  made has " << made.size() << " ints\n";

    // std::move only asks for a move. A const object cannot be moved from,
    // so this quietly copies.
    const std::vector<int> fixed{1, 2};
    std::vector<int> d = std::move(fixed);
    std::cout << "std::move on a const vector copied it: fixed has "
              << fixed.size() << ", d has " << d.size() << '\n';
}
```

**Rust**

```rust
// Prints when it is dropped, to show that a moved value is dropped once.
struct Token(&'static str);

impl Drop for Token {
    fn drop(&mut self) {
        println!("  drop {}", self.0);
    }
}

// Takes ownership of v. When the function ends, v is dropped.
fn keep(v: Vec<i32>) -> usize {
    v.len()
}

// Takes ownership, changes the vector, and gives it back.
fn with_six(mut v: Vec<i32>) -> Vec<i32> {
    v.push(6);
    v
}

fn main() {
    let a = vec![1, 2, 3];
    let b = a.clone(); // a copy, and you have to ask for it
    let c = a; // a move: c owns the heap block, and a cannot be used again
    println!("clone then move: b = {b:?}, c = {c:?}");

    let p = Box::new(7);
    let q = p;
    println!("after the move, q = {q}, and p is gone");

    let big = vec![1, 2, 3, 4, 5];
    println!("keep(big.clone()) = {}", keep(big.clone()));
    println!("keep(big) = {}, and big is gone", keep(big));

    let v = with_six(vec![4, 5]);
    println!("with_six gave back {v:?}");

    // To move out of something that must stay usable, leave a value behind.
    let mut slot = Some(String::from("job"));
    let job = slot.take();
    let mut list = vec![1, 2];
    let old = std::mem::take(&mut list);
    println!("take: slot = {slot:?}, job = {job:?}, list = {list:?}, old = {old:?}");

    println!("a value is dropped by its last owner, once:");
    {
        let first = Token("token");
        let second = first;
        println!("  moved from first to second");
        drop(second);
        println!("  first's scope ends, and nothing is dropped twice");
    }

    // Box::into_raw gives up ownership and returns a raw pointer. Nothing owns
    // the value until Box::from_raw takes it back, and forgetting to would
    // leak it.
    let boxed = Box::new(String::from("on the heap"));
    let raw: *mut String = Box::into_raw(boxed);
    let back = unsafe { Box::from_raw(raw) };
    println!("into_raw, then from_raw: {back}");
}
```

*C:*

```text
after message_move(&b, &a): a = (nothing), b = hello
freed both, and the block was freed once
```

*C++:*

```text
copy then move: a has 0, b has 3, c has 3
after the move, p is empty and *q = 7
passing by value:
  copied 5 ints
  moved, copied 0 ints
returning a local:
  made has 3 ints
std::move on a const vector copied it: fixed has 2, d has 2
```

*Rust:*

```text
clone then move: b = [1, 2, 3], c = [1, 2, 3]
after the move, q = 7, and p is gone
keep(big.clone()) = 5
keep(big) = 5, and big is gone
with_six gave back [4, 5, 6]
take: slot = None, job = Some("job"), list = [], old = [1, 2]
a value is dropped by its last owner, once:
  moved from first to second
  drop token
  first's scope ends, and nothing is dropped twice
into_raw, then from_raw: on the heap
```

**C: copy the pointer, then forget it.** C cannot move anything. `message_move` copies
the pointer and sets the old one to `NULL`, which is the whole idea of a move: the new
owner has the block, and the old one has nothing to free. Writing `b = a` instead leaves
two owners, and freeing both frees the block twice.

**C++: `std::move` asks for a move.** `b = a` calls the copy constructor and copies
every item. `std::move(a)` does not move anything by itself; it is a cast that lets the
compiler pick the **move constructor**, which takes `a`'s heap block and leaves `a`
empty. The moved-from object is still there, and it is still destroyed at the end of its
scope, so it must be left in a state its destructor can handle: "valid but unspecified",
as the standard puts it. The standard guarantees two cases: a vector moved into a new
vector is left empty, and a moved-from `std::unique_ptr` is null. A moved-from
`std::string` is empty with GCC's library, but the standard does not promise it, so
assign a new value before you use one. Copying a `unique_ptr` does not compile:
`error: use of deleted function 'std::unique_ptr<_Tp, _Dp>::unique_ptr(const std::unique_ptr<_Tp, _Dp>&)'`.

**Passing and returning.** `keep` takes its argument by value, so the caller decides:
`keep(big)` copies, and `keep(std::move(big))` moves. Returning a local usually does
neither, because the compiler builds the local directly in the caller's variable. Where
it cannot, `return` moves. And `std::move` on a `const` object silently copies, because
a move has to change its source.

**Rust: a move is the default.** `let c = a;` moves the vector, and the compiler then
refuses any use of `a`: `error[E0382]: borrow of moved value: 'a'`, with notes pointing
at the move. Passing to a function moves too, so `keep(big)` takes the vector, and `big`
is gone. A moved value is dropped once, by its last owner: the compiler tracks which
variables still own something and drops only those, which is why the token is dropped by
`drop(second)` and not again at the end of `first`'s scope. Numbers, `bool`, `char`,
shared references, and types marked `Copy` are copied instead, because copying them is
as cheap as moving and they own nothing. Anything else is copied only with an explicit
`clone()`.

**Moving out of a place that must stay valid.** A field or a slot in a vector cannot be
left empty, so Rust gives you ways to leave something behind: `Option::take` leaves
`None`, `std::mem::take` leaves the type's default, such as an empty `Vec`, and
`std::mem::replace` leaves a value you choose. They are what C's `from->text = NULL`
does, checked by the compiler.

**Leaving ownership altogether.** `Box::into_raw` gives up ownership and returns a raw
pointer. Nothing frees that value until `Box::from_raw`, inside `unsafe`, takes it back.
Rust 1.99 added the same for vectors, `Vec::into_parts` and `Vec::from_parts`, along
with `Box::into_non_null`. These exist for handing memory to C and back, which chapter
22 does.

The two languages mean different things by "move". In C++, a move is a function that
runs and leaves an empty shell behind, which you can still use by mistake. In Rust, it
is a transfer the compiler tracks, and the old name simply stops working.

---

## 10.7 Shared ownership and cycles

Most values have one owner, and `std::unique_ptr` and `Box` give a heap value exactly
that. Some values really are shared, such as a document open in two windows or a node
with two parents. **Reference counting** lets them have several owners: each owner adds
one to a count, letting go subtracts one, and whoever brings it to zero frees the value.
C does it by hand, and C++ and Rust do it for you:

**C**

```c
#include <stdio.h>
#include <stdlib.h>

// Reference counting by hand: each node counts its owners and frees itself
// when the last one lets go. C++'s shared_ptr and Rust's Rc do this for you.
typedef struct Node {
    const char *name;
    int owners;
    struct Node *next; // an owning link: it counts as an owner
} Node;

Node *node_new(const char *name) {
    Node *n = malloc(sizeof *n);
    if (n == NULL) {
        exit(1);
    }
    n->name = name;
    n->owners = 1;
    n->next = NULL;
    printf("  make %s\n", name);
    return n;
}

Node *node_retain(Node *n) {
    n->owners++;
    return n;
}

void node_release(Node *n) {
    if (n == NULL || --n->owners > 0) {
        return;
    }
    printf("  drop %s\n", n->name);
    node_release(n->next);
    free(n);
}

int main(void) {
    printf("two owners:\n");
    Node *doc = node_new("doc");
    Node *also = node_retain(doc);
    printf("  owners: %d\n", doc->owners);
    node_release(doc);
    printf("  one owner let go, owners: %d\n", also->owners);
    node_release(also);

    printf("a cycle of owning links:\n");
    Node *a = node_new("a");
    Node *b = node_new("b");
    a->next = node_retain(b);
    b->next = node_retain(a);
    Node *watch = a; // not an owner: just a way to look
    node_release(a);
    node_release(b);
    printf("  both owners let go, and a and b still count %d owner each\n",
           watch->owners);
    printf("  break the cycle by hand:\n");
    Node *next = watch->next;
    watch->next = NULL;
    node_release(next);
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <memory>
#include <string>
#include <utility>

struct Node {
    std::string name;
    std::shared_ptr<Node> next; // owns what it points to
    std::weak_ptr<Node> back;   // does not

    explicit Node(std::string n) : name(std::move(n)) {
        std::cout << "  make " << name << '\n';
    }
    ~Node() { std::cout << "  drop " << name << '\n'; }
};

const char *yes_no(bool flag) { return flag ? "yes" : "no"; }

int main() {
    std::cout << "one owner:\n";
    {
        auto report = std::make_unique<Node>("report");
    }

    std::cout << "two owners:\n";
    {
        auto doc = std::make_shared<Node>("doc");
        auto also = doc;
        std::cout << "  owners: " << doc.use_count() << '\n';
        doc.reset();
        std::cout << "  one owner let go, owners: " << also.use_count() << '\n';
    }

    std::cout << "a weak pointer does not own:\n";
    std::weak_ptr<Node> weak;
    {
        auto cache = std::make_shared<Node>("cache");
        weak = cache;
        std::cout << "  weak can reach cache: " << yes_no(!weak.expired()) << '\n';
    }
    std::cout << "  weak can reach cache: " << yes_no(!weak.expired()) << '\n';

    std::cout << "a cycle of owning links:\n";
    std::weak_ptr<Node> watch;
    {
        auto a = std::make_shared<Node>("a");
        auto b = std::make_shared<Node>("b");
        a->next = b;
        b->next = a;
        watch = a;
    }
    std::cout << "  a and b went out of scope, and a still has "
              << watch.use_count() << " owner\n";
    std::cout << "  break the cycle by hand:\n";
    watch.lock()->next.reset();

    std::cout << "an owning link one way, a weak link back:\n";
    {
        auto a = std::make_shared<Node>("a");
        auto b = std::make_shared<Node>("b");
        a->next = b;
        b->back = a;
        std::cout << "  b's back link reaches " << b->back.lock()->name << '\n';
    }
}
```

**Rust**

```rust
use std::cell::RefCell;
use std::rc::{Rc, Weak};

// Rc counts owners. RefCell lets shared nodes change their links.
struct Node {
    name: &'static str,
    next: Option<Rc<RefCell<Node>>>, // owns what it points to
    back: Weak<RefCell<Node>>,       // does not
}

fn node(name: &'static str) -> Node {
    println!("  make {name}");
    Node {
        name,
        next: None,
        back: Weak::new(),
    }
}

fn make(name: &'static str) -> Rc<RefCell<Node>> {
    Rc::new(RefCell::new(node(name)))
}

impl Drop for Node {
    fn drop(&mut self) {
        println!("  drop {}", self.name);
    }
}

fn yes_no(flag: bool) -> &'static str {
    if flag { "yes" } else { "no" }
}

fn main() {
    println!("one owner:");
    {
        let _report = Box::new(node("report"));
    }

    println!("two owners:");
    {
        let doc = make("doc");
        let also = Rc::clone(&doc);
        println!("  owners: {}", Rc::strong_count(&doc));
        drop(doc);
        println!("  one owner let go, owners: {}", Rc::strong_count(&also));
    }

    println!("a weak pointer does not own:");
    let weak;
    {
        let cache = make("cache");
        weak = Rc::downgrade(&cache);
        let reach = weak.upgrade().is_some();
        println!("  weak can reach cache: {}", yes_no(reach));
    }
    let reach = weak.upgrade().is_some();
    println!("  weak can reach cache: {}", yes_no(reach));

    println!("a cycle of owning links:");
    let watch;
    {
        let a = make("a");
        let b = make("b");
        a.borrow_mut().next = Some(Rc::clone(&b));
        b.borrow_mut().next = Some(Rc::clone(&a));
        watch = Rc::downgrade(&a);
    }
    let owners = watch.strong_count();
    println!("  a and b went out of scope, and a still has {owners} owner");
    println!("  break the cycle by hand:");
    if let Some(a) = watch.upgrade() {
        a.borrow_mut().next = None;
    }

    println!("an owning link one way, a weak link back:");
    {
        let a = make("a");
        let b = make("b");
        a.borrow_mut().next = Some(Rc::clone(&b));
        b.borrow_mut().back = Rc::downgrade(&a);
        if let Some(back) = b.borrow().back.upgrade() {
            println!("  b's back link reaches {}", back.borrow().name);
        }
    }
}
```

*C:*

```text
two owners:
  make doc
  owners: 2
  one owner let go, owners: 1
  drop doc
a cycle of owning links:
  make a
  make b
  both owners let go, and a and b still count 1 owner each
  break the cycle by hand:
  drop b
  drop a
```

*C++ and Rust:*

```text
one owner:
  make report
  drop report
two owners:
  make doc
  owners: 2
  one owner let go, owners: 1
  drop doc
a weak pointer does not own:
  make cache
  weak can reach cache: yes
  drop cache
  weak can reach cache: no
a cycle of owning links:
  make a
  make b
  a and b went out of scope, and a still has 1 owner
  break the cycle by hand:
  drop b
  drop a
an owning link one way, a weak link back:
  make a
  make b
  b's back link reaches a
  drop a
  drop b
```

**One owner is the default.** `std::unique_ptr<T>` and `Box<T>` are one pointer each, 8
bytes on a 64-bit machine, and free their value when they go out of scope. Reach for
them first.

**A shared pointer keeps the count next to the value.** `std::shared_ptr` points at both
the value and a **control block** that holds the counts, so it is 16 bytes, and
`std::make_shared` puts the value and its control block in one allocation instead of
two. Rust's `Rc` is 8 bytes, a pointer to one block that holds the counts and the value.
`use_count()` and `Rc::strong_count` report the count, and the value is freed when it
reaches zero, which is why `doc` is dropped as soon as `also`, the last owner, goes.

**Threads need a different count.** The standard requires that `std::shared_ptr`'s count
can be updated from several threads at once, so it uses atomic instructions, which cost
a little more everywhere. Rust has two types: `Rc`, whose count is a plain number, and
`Arc`, whose count is atomic. The compiler stops an `Rc` from reaching another thread:
`error[E0277]: 'Rc<i32>' cannot be sent between threads safely`. Chapter 16 uses `Arc`.

**Shared values cannot be changed through `Rc` alone.** `Rc<T>` hands out only shared
references, because many owners writing at once is exactly what the borrow rules forbid.
`RefCell<T>` moves those rules to run time: `borrow_mut()` checks that nobody else is
borrowing, and panics if somebody is, with `RefCell already borrowed`. `Rc<RefCell<T>>`
is the usual way to share something that changes, and `Cell<T>` is a lighter version for
small `Copy` values. C++ has no such check: any owner of a `shared_ptr<T>` can change
the value.

**A weak pointer does not own.** `std::weak_ptr` and `Weak` point at a shared value
without counting as owners. To use the value, you ask for an owner back, with `lock()`
or `upgrade()`, and get nothing if the value is gone. That is how `weak` can tell that
`cache` was freed, which C++ can also ask directly with `expired()`.

**A cycle of owners is never freed.** When `a` owns `b` and `b` owns `a`, both counts
stay at 1 after the variables go out of scope, and nothing will ever bring them to zero.
The example breaks the cycle by hand. The better fix is to decide which way ownership
runs, as in a tree, where parents own their children and children point back with a weak
link, which is exercise 10.3. The tools in section 10.10 see these leaks. Leave a cycle
of two such nodes unbroken, and AddressSanitizer reports the C++ version as two
`Indirect leak` entries, while Valgrind reports the Rust version as one block definitely
lost and one indirectly lost.

**In Rust, a leak is safe.** `Rc` cycles, `std::mem::forget`, and `Box::leak`, which
turns a box into a reference that lasts until the program ends, all compile without
`unsafe`. Rust's promise of memory safety rules out use after free, double free, and
data races. It does not rule out holding memory for too long.

**C counts by hand.** `node_retain` and `node_release` are the whole of reference
counting. Real C libraries do the same: CPython's C API has `Py_INCREF` and `Py_DECREF`,
which is how Python itself frees most objects, and GLib has `g_object_ref` and
`g_object_unref`. Forget one release and the value leaks. Release once too often and it
is freed while still in use.

---

## 10.8 Borrowing and lifetimes

A **borrow** uses a value without owning it: a C pointer, a C++ reference,
`std::string_view` or `std::span`, or a Rust `&T` or `&mut T`. All three languages
borrow constantly, because copying would be slow and moving would take the value away.
Only Rust checks that a borrow ends before its owner frees the value:

**C**

```c
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Returns a or b, whichever is longer. The result points into one of them, so
// it is good only as long as both are. Only this comment says so.
const char *longest(const char *a, const char *b) {
    return strlen(b) > strlen(a) ? b : a;
}

// Walks the words of a string without copying them. It borrows the text, so
// the text must outlive it.
typedef struct {
    const char *rest;
} Words;

// Finds the next word and points *start at it. Returns false at the end.
bool words_next(Words *w, const char **start, size_t *length) {
    w->rest += strspn(w->rest, " ");
    if (*w->rest == '\0') {
        return false;
    }
    *start = w->rest;
    *length = strcspn(w->rest, " ");
    w->rest += *length;
    return true;
}

int main(void) {
    char a[] = "apple";
    char b[] = "fig";
    const char *result = longest(a, b);
    printf("longest(\"%s\", \"%s\") = %s\n", a, b, result);

    const char *text = "the quick brown fox";
    printf("words of \"%s\":", text);
    Words words = {text};
    const char *start;
    size_t length;
    while (words_next(&words, &start, &length)) {
        printf(" [%.*s]", (int)length, start);
    }
    printf("\n");

    // The words are pointers into text, not copies of it.
    Words again = {text};
    words_next(&again, &start, &length);
    if (words_next(&again, &start, &length)) {
        printf("\"%.*s\" starts %td bytes into the text\n", (int)length, start,
               start - text);
    }
    return 0;
}
```

**C++**

```cpp
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

// Returns a or b, whichever is longer. A string_view is a pointer and a
// length that borrows someone else's characters, so the result is good only
// as long as both strings are. The compiler does not check that.
std::string_view longest(std::string_view a, std::string_view b) {
    return b.size() > a.size() ? b : a;
}

// Walks the words of a string without copying them. It borrows the text.
class Words {
  public:
    explicit Words(std::string_view text) : rest_(text) {}

    std::optional<std::string_view> next() {
        std::size_t start = rest_.find_first_not_of(' ');
        if (start == std::string_view::npos) {
            return std::nullopt;
        }
        rest_.remove_prefix(start);
        std::size_t end = rest_.find(' ');
        std::string_view word = rest_.substr(0, end);
        rest_.remove_prefix(word.size());
        return word;
    }

  private:
    std::string_view rest_;
};

int main() {
    std::string a = "apple";
    std::string b = "fig";
    std::string_view result = longest(a, b);
    std::cout << "longest(\"" << a << "\", \"" << b << "\") = " << result
              << '\n';

    const std::string text = "the quick brown fox";
    std::cout << "words of \"" << text << "\":";
    Words words(text);
    while (auto word = words.next()) {
        std::cout << " [" << *word << ']';
    }
    std::cout << '\n';

    // The words are views into text, not copies of it.
    Words again(text);
    again.next();
    if (auto second = again.next()) {
        std::cout << '"' << *second << "\" starts "
                  << second->data() - text.data() << " bytes into the text\n";
    }
}
```

**Rust**

```rust
// The lifetime 'a says: the result borrows from a or b, so it can be used
// only while both are alive. The compiler checks every caller.
fn longest<'a>(a: &'a str, b: &'a str) -> &'a str {
    if b.len() > a.len() { b } else { a }
}

// Walks the words of a string without copying them. Words<'a> borrows the
// text, and the compiler will not let the text go away first.
struct Words<'a> {
    rest: &'a str,
}

impl<'a> Iterator for Words<'a> {
    type Item = &'a str;

    fn next(&mut self) -> Option<&'a str> {
        let rest = self.rest.trim_start_matches(' ');
        if rest.is_empty() {
            return None;
        }
        let end = rest.find(' ').unwrap_or(rest.len());
        let (word, after) = rest.split_at(end);
        self.rest = after;
        Some(word)
    }
}

// One reference goes in and one comes out, so the compiler can tell they share
// a lifetime. Words<'_> shows that the result borrows. Plain Words would get a
// mismatched_lifetime_syntaxes warning.
fn words(text: &str) -> Words<'_> {
    Words { rest: text }
}

fn main() {
    let a = String::from("apple");
    let b = String::from("fig");
    let result = longest(&a, &b);
    println!("longest(\"{a}\", \"{b}\") = {result}");

    let text = String::from("the quick brown fox");
    print!("words of \"{text}\":");
    for word in words(&text) {
        print!(" [{word}]");
    }
    println!();

    // The words are slices of text, not copies of it.
    if let Some(second) = words(&text).nth(1) {
        let offset = second.as_ptr() as usize - text.as_ptr() as usize;
        println!("\"{second}\" starts {offset} bytes into the text");
    }

    // A borrow lasts until its last use, not to the end of the scope, so the
    // push is allowed: first is not used after it.
    let mut scores = vec![90, 85];
    let first = &scores[0];
    println!("first = {first}");
    scores.push(70);
    println!("after the push, scores = {scores:?}");
}
```

*C and C++:*

```text
longest("apple", "fig") = apple
words of "the quick brown fox": [the] [quick] [brown] [fox]
"quick" starts 4 bytes into the text
```

*Rust:*

```text
longest("apple", "fig") = apple
words of "the quick brown fox": [the] [quick] [brown] [fox]
"quick" starts 4 bytes into the text
first = 90
after the push, scores = [90, 85, 70]
```

The output proves the words are borrowed: `"quick"` starts 4 bytes into the text, so it
points into the string rather than holding a copy. All three versions work. The
difference is what happens when someone uses them wrongly.

**Rust's two rules.** The **borrow checker** is the part of the compiler that enforces
them. At any moment a value can have either any number of shared borrows, `&T`, or
exactly one mutable borrow, `&mut T`, and never both. And no borrow may outlive the
value it borrows. Chapter 9 showed the first rule stopping aliasing bugs. This chapter
is about the second.

**A lifetime names how long a borrow is good.** `longest<'a>` says the result borrows
from `a` or `b`, so it can be used only while both are alive. Leave the lifetime out and
the compiler cannot guess which input the result comes from:
`error[E0106]: missing lifetime specifier`, with the help line
`this function's return type contains a borrowed value, but the signature does not say whether it is borrowed from 'a' or 'b'`.
With the lifetime in place, each caller is checked. Keep the result past the end of
`b`'s block and the program does not compile:

```rust
let a = String::from("apple");
let result;
{
    let b = String::from("fig");
    result = longest(&a, &b);
}
println!("{result}"); // error[E0597]: `b` does not live long enough
```

C and C++ accept the same code. `result` then points at freed memory, and the program's
behavior is undefined. A lifetime never makes anything live longer. It only describes,
and lets the compiler check, what the code already does.

**Most lifetimes are inferred.** The compiler fills them in for the common shapes, a
process called **lifetime elision**. Each reference parameter gets its own lifetime. If
there is exactly one input lifetime, the result gets it. A method that takes `&self` or
`&mut self` gives the result `self`'s lifetime. So `fn words(text: &str) -> Words<'_>`
needs no names: one reference goes in, so the result borrows from it. The `'_` stands
for that inferred lifetime and shows the reader that `Words` borrows. Writing plain
`Words` still compiles, but since Rust 1.89 it draws a warning,
`hiding a lifetime that's elided elsewhere is confusing`, which suggests `Words<'_>`,
and the book's `-D warnings` turns the warning into an error.

**A struct that borrows carries a lifetime.** `Words<'a>` holds a `&'a str`, so a
`Words` can never outlive the text it walks. That is what makes the iterator cheap: each
word is a slice of the original text, and the compiler guarantees the text stays put
while any word is in use. The C and C++ versions do the same thing, and their comments
say what Rust's types enforce.

**A borrow lasts until its last use.** `first` borrows `scores`, but the borrow ends at
the `println!` that last reads it, so the `push` after it is allowed. Before Rust 2018,
borrows lasted to the end of the block and this code did not compile. Swap the two
lines, so that `first` is read after the `push`, and you get chapter 9's `error[E0502]`,
which is right: the push may move the vector's items, and `first` would dangle. Moving a
value while it is borrowed fails the same way:
`error[E0505]: cannot move out of 's' because it is borrowed`.

**C++ checks a little, by warning.** `std::string_view` and `std::span` are borrows with
no checking at all. GCC 13 warns about one common case, a reference bound to the result
of a function whose argument was a temporary:
`warning: possibly dangling reference to a temporary [-Wdangling-reference]`. It says
nothing about `std::string_view view = make();`, where `make` returns a `std::string`
that is destroyed at the end of the line. Clang does catch that one:
`warning: object backing the pointer will be destroyed at the end of the full-expression [-Wdangling-gsl]`.
AddressSanitizer catches it at run time as a `heap-use-after-free`. C has only the
comment.

---

## 10.9 The garbage-collected languages: cleanup blocks

Go, Python, JavaScript, TypeScript, Dart, and Kotlin never ask you to free memory. A
**garbage collector** finds objects that nothing can reach and frees them at a time of
its choosing. All six collect cycles too, so the `a` and `b` that leak in C++ and Rust
are freed here. What a collector cannot do is close a file at the right moment, because
it does not know when you are done with the file, and it may not run for a long time. So
every one of these languages has a way to say "run this when the block ends, however it
ends". Here is section 10.4's program in Go and Python:

**Go**

```go
package main

import (
	"errors"
	"fmt"
)

// File is a pretend file that prints when it opens and closes. A real
// *os.File's Close also returns an error; see section 10.9.
type File struct {
	name string
}

func Open(name string, fails bool) (*File, error) {
	if fails {
		return nil, fmt.Errorf("cannot open %s", name)
	}
	fmt.Printf("  open %s\n", name)
	return &File{name}, nil
}

func (f *File) Close() {
	fmt.Printf("  close %s\n", f.name)
}

// Each defer runs when the function returns, last deferred first.
func runJob(outputFails, copyFails bool) error {
	input, err := Open("input", false)
	if err != nil {
		return err
	}
	defer input.Close()

	output, err := Open("output", outputFails)
	if err != nil {
		return err
	}
	defer output.Close()

	if copyFails {
		return errors.New("disk full")
	}
	fmt.Println("  copy input to output")
	return nil
}

func main() {
	jobs := []struct {
		title       string
		outputFails bool
		copyFails   bool
	}{
		{"everything works", false, false},
		{"opening output fails", true, false},
		{"copying fails", false, true},
	}
	for i, job := range jobs {
		number := i + 1
		fmt.Printf("job %d: %s\n", number, job.title)
		if err := runJob(job.outputFails, job.copyFails); err != nil {
			fmt.Printf("  job %d failed: %v\n", number, err)
		} else {
			fmt.Printf("  job %d done\n", number)
		}
	}
}
```

**Python**

```python
from types import TracebackType


class File:
    """A pretend file. `with` calls __enter__ at the start and __exit__ at the end."""

    def __init__(self, name: str, fails: bool = False) -> None:
        if fails:
            raise OSError(f"cannot open {name}")
        print(f"  open {name}")
        self.name = name

    def __enter__(self) -> "File":
        return self

    def __exit__(
        self,
        kind: type[BaseException] | None,
        error: BaseException | None,
        traceback: TracebackType | None,
    ) -> None:
        print(f"  close {self.name}")


def run_job(output_fails: bool, copy_fails: bool) -> None:
    # However the block ends, each file that opened is closed, in the
    # reverse order.
    with File("input"), File("output", output_fails):
        if copy_fails:
            raise OSError("disk full")
        print("  copy input to output")


def main() -> None:
    jobs = [
        ("everything works", False, False),
        ("opening output fails", True, False),
        ("copying fails", False, True),
    ]
    for number, (title, output_fails, copy_fails) in enumerate(jobs, start=1):
        print(f"job {number}: {title}")
        try:
            run_job(output_fails, copy_fails)
            print(f"  job {number} done")
        except OSError as error:
            print(f"  job {number} failed: {error}")


if __name__ == "__main__":
    main()
```

JavaScript has no cleanup syntax that Node 22 can run, so each resource gets its own
`try`/`finally`. TypeScript and Dart write `runJob` the same way:

```js
function runJob(outputFails, copyFails) {
  const input = new File("input");
  try {
    const output = new File("output", outputFails);
    try {
      if (copyFails) throw new Error("disk full");
      console.log("  copy input to output");
    } finally {
      output.close();
    }
  } finally {
    input.close();
  }
}
```

Kotlin's `use` takes a block and closes the file after it:

```kotlin
fun runJob(outputFails: Boolean, copyFails: Boolean) {
    File("input").use {
        File("output", outputFails).use {
            if (copyFails) throw IOException("disk full")
            println("  copy input to output")
        }
    }
}
```

All six print the same lines as C, C++, and Rust. The full programs are in
[examples/02-cleanup](examples/02-cleanup/).

**Go: `defer`.** A deferred call runs when the surrounding function returns, whether it
returns normally, with an error, or in a panic, and deferred calls run last first. Three
details catch people out. The arguments are evaluated when the `defer` statement runs,
not when the call does, so `defer show(n)` followed by `n = 2` still shows 1. A `defer`
belongs to the function, not to the block, so deferring a close inside a loop keeps
every file open until the function returns. Move the loop body into its own function to
close each file in turn. And a real `*os.File`'s `Close` returns an error, which
`defer f.Close()` throws away. For a file you only read, that is fine. For a file you
wrote, the error can mean the data never reached the disk, so call `Close` yourself and
check it.

**Python: `with`.** A `with` block calls the object's `__enter__` at the start and its
`__exit__` at the end, even when an exception leaves the block, and one `with` can open
several things, which close in the reverse order. `open` returns a file that works this
way. To write your own, either give a class those two methods, as `File` does, or write
a generator with `@contextlib.contextmanager` that wraps its `yield` in `try`/`finally`,
as exercise 10.1's solution does. `contextlib.ExitStack` handles a number of resources
you only know at run time.

CPython frees an object as soon as its reference count reaches zero, so a file you
forget to close usually closes when the last variable that refers to it goes. Many
programs rely on that, but the language does not promise it, and it does not happen
while the file is caught in a cycle. Python's development mode reports the files you
forgot. Run a function that calls `open("/etc/hostname")` and never closes the file
under `python3 -X dev`, and it prints
`ResourceWarning: unclosed file <_io.TextIOWrapper name='/etc/hostname' mode='r' encoding='utf-8'>`.
Cycles wait for the cycle collector, which runs every so often, or at once if you call
`gc.collect()`.

**JavaScript and TypeScript: `try`/`finally`, and `using` soon.** A `finally` block runs
however its `try` block ends, so nesting one per resource closes them in the reverse
order. The newer **explicit resource management** syntax does the nesting for you:
`using a = new Resource("a")` calls `a[Symbol.dispose]()` when the block ends, last
declared first. TypeScript already type-checks it, and the book's compiler flags accept
it, but Node 22 cannot run it: `SyntaxError: Unexpected identifier 'a'`. Compiling to an
older target such as ES2022 rewrites `using` into `try`/`finally` code that does run on
Node 22, but the book runs TypeScript files directly, so its examples use `finally`.

**Dart: `try`/`finally`.** Dart has no cleanup syntax, so the code looks like
JavaScript's. Its `Finalizer` class can run a callback some time after an object is
collected, but the program may end first. In a test program that attached one and then
exited, the callback never ran.

**Kotlin: `use`.** `use` is an extension function on `AutoCloseable` (and `Closeable`):
it runs the block, closes the object, and passes on any exception. If the block throws
and then `close` throws too, the second exception is attached to the first as a
**suppressed** exception rather than replacing it. A block that failed with
`work failed`, on a resource whose close failed with `close of log failed`, is caught as
`work failed` with `suppressed: [close of log failed]`. It is the same as Java's
`try`-with-resources.

**Finalizers are not destructors.** Every collected language can run code after an
object becomes unreachable: Go's `runtime.AddCleanup` (Go 1.24) and older
`runtime.SetFinalizer`, Python's `__del__` and `weakref.finalize`, JavaScript's
`FinalizationRegistry`, Dart's `Finalizer`, and Java's `Cleaner`, which Kotlin uses.
They run late, in no particular order, and sometimes never, so they are a safety net,
for things such as memory that C code owns, and not a way to close files.

**A collector can still leak.** Memory that something can reach is never freed, even if
the program will never use it again. A cache that only grows, a list of event listeners
nobody removes, or a global map of sessions all hold their contents forever. The fix is
the same as in C: decide who owns each entry and when it goes.

---

## 10.10 Finding leaks and memory bugs

The compiler catches a few memory bugs in C and C++. Two tools catch most of the rest
while the program runs, on Linux. Both work best with `-g`, which puts line numbers in
the reports. Here is a leak: assigning a second copy to `name` loses the only pointer to
the first.

```c
#include <stdlib.h>
#include <string.h>

char *copy(const char *text) {
    char *result = malloc(strlen(text) + 1);
    strcpy(result, text);
    return result;
}

int main(void) {
    char *name = copy("Ada");
    name = copy("Grace"); // the only pointer to "Ada" is lost
    free(name);
    return 0;
}
```

**AddressSanitizer** is built into GCC and Clang. Build with `-fsanitize=address` and
run the program as usual. It stops at the first use after free, double free, or
out-of-bounds access, and its leak checker, LeakSanitizer, reports leaks when the
program exits:

```text
$ cc -std=c17 -g -fsanitize=address -o leak leak.c && ./leak
=================================================================
==11149==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 4 byte(s) in 1 object(s) allocated from:
    #0 0x7f0e94afd9c7 in malloc ../../../../src/libsanitizer/asan/asan_malloc_linux.cpp:69
    #1 0x55ef6c0e0250 in copy /tmp/leakdemo/leak.c:5
    #2 0x55ef6c0e0288 in main /tmp/leakdemo/leak.c:11
```

The stack says where the leaked block was allocated: by `copy`, called from line 11. The
fix goes where the last pointer to it was lost: line 12 has to free the old name before
it stores the new copy. A use after free gets a report of its own,
`ERROR: AddressSanitizer: heap-use-after-free`, followed by the line that read the
memory and the lines that allocated and freed it.

**Valgrind** runs an ordinary build on a simulated processor that watches every
allocation and every memory access. It needs no special build, and it is much slower:

```text
$ cc -std=c17 -g -O0 -o leak leak.c && valgrind --leak-check=full ./leak
==11157== 4 bytes in 1 blocks are definitely lost in loss record 1 of 1
==11157==    at 0x4846828: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==11157==    by 0x1091D0: copy (leak.c:5)
==11157==    by 0x109208: main (leak.c:11)
```

**Definitely lost** means nothing points to the block any more. **Indirectly lost**
means the only pointers to it are inside other lost blocks, such as the strings of a
lost struct. Fix the definite leaks first, and the indirect ones usually go with them.
Valgrind works on any compiled program, so it finds the `Rc` cycle in a Rust binary too.

Use AddressSanitizer while you develop and in tests, since it is fast enough to leave
on. Use Valgrind when you cannot rebuild the program, or to check a build without the
sanitizer. Neither one proves a program is correct. They report what happened in the
runs you gave them, which is why exercise 10.2 has you run every test. Chapter 19 comes
back to both.

---

## 10.11 Why each language looks the way it does

- **C** came from a time when programs were small and memory was scarce, and it gives
  you the allocator and nothing else. That keeps the language simple and lets you manage
  memory in any scheme you like, such as arenas, pools, or reference counts. The price
  is that every owner, every borrow, and every cleanup path lives in your head and your
  comments.
- **C++** added constructors and destructors to C in the early 1980s, and RAII grew out
  of them: if a destructor always runs at the end of a scope, put every release there.
  Exceptions made it essential, since they can leave a function from almost any line.
  Move semantics arrived in C++11, so that returning a big vector or passing ownership
  of a `unique_ptr` no longer needed a copy. The compiler still trusts every raw pointer
  and reference, so dangling ones remain your problem.
- **Rust** keeps RAII and moves, makes moving the default, and adds the borrow checker,
  so that a reference can never outlive its owner. That rules out use after free and
  double free in safe code, with no garbage collector, and the checking costs nothing at
  run time. The cost is paid at compile time: some correct programs need restructuring
  before the compiler accepts them, and shared, changing data has to say so with
  `Rc<RefCell<T>>` or `Arc<Mutex<T>>`.
- **Go** was designed for servers, and its garbage collector runs alongside the program
  to keep pauses short. `defer` handles the resources the collector cannot, and it
  belongs to functions rather than blocks, which keeps the rule to one sentence.
- **Python** has counted references since the start, and CPython's prompt freeing became
  something programs depend on. The cycle collector came later, in Python 2.0, to handle
  what counting misses. `with` arrived in Python 2.5, so that closing a file properly no
  longer took a `try`/`finally` block.
- **JavaScript** ran in a browser for a single page, where leaks rarely mattered and the
  page was soon closed. It had no destructors and no cleanup syntax beyond `finally`.
  Servers on Node and long-lived web apps made resource cleanup matter, and explicit
  resource management is the language's answer.
- **TypeScript** adopts JavaScript features as they are standardized, so it supported
  `using` before Node could run it. It cannot change when memory is freed, only check
  the code that frees other things.
- **Dart** uses a garbage collector tuned for Flutter, where many short-lived objects
  are made every frame. It has never added cleanup syntax. In a Flutter app, the
  framework calls each widget state's `dispose` method when the widget goes away, which
  covers most resources an app holds.
- **Kotlin** inherited the JVM's collector and Java's `AutoCloseable`, and made `use` an
  ordinary extension function rather than syntax, the same way it builds `apply`, `let`,
  and `run` from lambdas.

---

## 10.12 Pitfalls

| Language | Mistake | What happens |
| --- | --- | --- |
| C | An early `return` between `malloc` and `free` | A leak; use one exit with `goto` cleanup labels |
| C | Freeing a struct before the blocks it points to | The blocks leak, since nothing points to them now |
| C | `p = malloc(...)` when `p` already owns a block | The old block leaks |
| C | Setting a pointer to `NULL` as if that freed it | A leak: `NULL` frees nothing |
| C | Using memory after `free` | Undefined behavior; often seems to work; AddressSanitizer: `heap-use-after-free` |
| C, C++ | `for (n = list; n; n = n->next) free(n);` | Reads `n->next` after freeing `n`; save it first |
| C++ | A class that frees in its destructor, copied by accident | Two destructors free one resource: `free(): double free detected in tcache 2` |
| C++ | Throwing from a destructor | `std::terminate`: `terminate called after throwing an instance of ...` |
| C++ | Using an object after `std::move` | Valid but unspecified, except for a few types such as `std::unique_ptr` |
| C++ | `std::move` on a `const` object | Copies quietly |
| C++ | Two `shared_ptr`s that own each other | Neither is ever freed; make one a `weak_ptr` |
| C++ | A `std::string_view` of a temporary string | Dangles at the end of the line; only Clang warns |
| Rust | `let _ = make_guard();` for a guard you meant to keep | Dropped at once; for a lock, the compiler refuses: `non-binding let on a synchronization lock` |
| Rust | Using a value after moving it | `error[E0382]: borrow of moved value` |
| Rust | Returning a reference with no lifetime to tie it to | `error[E0106]: missing lifetime specifier` |
| Rust | `Rc` links in both directions | A leak, with no error; make one direction `Weak` |
| Rust | Two `borrow_mut()` calls on one `RefCell` at once | A panic at run time: `RefCell already borrowed` |
| Go | `defer f.Close()` inside a loop | Every file stays open until the function returns |
| Go | `defer f.Close()` on a file you wrote | The close error, which may mean lost data, is ignored |
| Python | `f = open(...)` without `with` | Usually closed when `f` goes, but not promised; `-X dev` warns |
| JavaScript, TypeScript | `using` on Node 22 | `SyntaxError`; use `try`/`finally` |
| All with a collector | A cache or listener list that only grows | A leak the collector cannot see |
| All with a collector | Relying on a finalizer to close a file | It may run late or never |

---

## 10.13 Idiom check

**C: one owner per block, written down.** Pair every allocating function with a freeing
one, and say in a comment who owns each pointer a function takes or returns. Use one
exit with `goto` cleanup labels in any function that acquires more than one thing.
Accept `NULL` in your free functions, set pointers to `NULL` after freeing, and run your
tests under AddressSanitizer.

**C++: the rule of zero.** Hold every resource in an object whose destructor releases
it: containers and `std::string` for memory, `std::unique_ptr` for one heap object,
`std::fstream` and `std::lock_guard` for files and locks. Then write no destructors,
copies, or moves yourself. Reach for `std::shared_ptr` only when ownership really is
shared, break cycles with `std::weak_ptr`, and pass plain references or
`std::string_view` to code that only borrows.

**Rust: own, borrow, and clone on purpose.** Let values have one owner, pass `&T` and
`&mut T` to functions that only use them, and move values into functions that keep them.
Use `Box` for one heap value, `Rc` or `Arc` only for shared ownership, `Weak` for back
links, and `RefCell` or `Mutex` only where shared data must change. When a borrow error
appears, restructure the code before reaching for `clone()`, and write `'_` wherever a
returned type borrows.

**Go: `defer` right after the open.** Write `defer f.Close()` on the line after a
successful open, so every return closes the file. Put the work for each file in its own
function when you open files in a loop, and check `Close`'s error for files you write.

**Python: `with` for anything that closes.** Open files, sockets, and locks in a `with`
block, write context managers with `@contextlib.contextmanager`, and use
`contextlib.ExitStack` for a list of resources. Run your tests with `-X dev` to see the
files you forgot.

**JavaScript and TypeScript: `try`/`finally` until you can use `using`.** Close each
resource in its own `finally`, and give your own resource types a `close` method. When
your runtime supports explicit resource management, implement `[Symbol.dispose]` and
switch to `using`.

**Dart: `try`/`finally`, and close what you open.** Close files, sockets, and stream
subscriptions in `finally` blocks, and leave `Finalizer` to code that wraps native
memory.

**Kotlin: `use` for every `AutoCloseable`.** Make your resource types implement
`AutoCloseable`, open them with `use`, and nest one `use` per resource.

---

## 10.14 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Files that always close](exercises/01-always-close/) | Cleanup on every path in each language's style, with a leak that shows up as `too many open files` |
| 2 | [Find the leaks](exercises/02-find-the-leaks/) | Finding five leaks in a C program with AddressSanitizer and Valgrind, and fixing them |
| 3 | [Folders that know their parent](exercises/03-parent-links/) | A tree that owns its children and points back at its parents, in C, C++, and Rust |

```sh
python3 tools/check.py 10                     # your answers, every language
python3 tools/check.py 10 --lang cpp,rust     # some languages
python3 tools/check.py 10 --solutions         # the reference answers
```

Exercise 2 is C only and exercise 3 is C, C++, and Rust, so the checker skips them for
the other languages.

**Stretch:** write exercise 3 in Python, Go, or Kotlin. Without destructors, how would
you count the folders that were freed, and why can't the count be exact? Then add a
cache to the Rust version that remembers the last 5 folders looked up by `path`. Should
it hold `Rc` or `Weak`? Try both, and watch what `rm` reports.

---

Previous: [Chapter 9: Memory model](../09-memory-model/) ·
Next: [Chapter 11: Error handling](../11-error-handling/)
