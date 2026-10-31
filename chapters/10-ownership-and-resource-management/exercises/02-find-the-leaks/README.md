# Exercise 10.2: Find the leaks

This exercise is in C only. [c/main.c](c/main.c) is a finished playlist program that
prints the right thing for every test except its last line. It has five memory leaks,
and your job is to find each one with Valgrind or AddressSanitizer and fix it.

The program reads one command per line:

| Command | Effect |
| --- | --- |
| `add TITLE ARTIST` | Add a song at the end and print `added TITLE` |
| `remove TITLE` | Remove a song and print `removed TITLE` |
| `rename OLD NEW` | Change a song's title and print `renamed OLD to NEW` |
| `show` | Print `1. TITLE by ARTIST` for each song, or `(empty)` |
| `clear` | Remove every song and print `cleared N songs` |

Titles are unique, so adding or renaming to a title already in the playlist prints
`error: "TITLE" is already in the playlist`. Removing or renaming a song that is not
there prints `error: no song "TITLE"`. When the input ends, the program empties the
playlist and prints `blocks still allocated: N`.

Every allocation in the program goes through `counted_malloc` and `counted_free`,
which keep that count. Don't change them, and don't allocate around them. For the
starter, test 1 ends with `blocks still allocated: 12`. When you have fixed all five
leaks, every test ends with `blocks still allocated: 0`.

```sh
python3 tools/check.py 10 --lang c
```

## Finding the leaks

The count tells you that memory leaked, but not where. The tools tell you where each
leaked block was allocated. These commands are for Linux; the count works everywhere.

**AddressSanitizer** is built into GCC and Clang. Its leak checker runs when the
program exits:

```sh
cd chapters/10-ownership-and-resource-management/exercises/02-find-the-leaks
cc -std=c17 -g -fsanitize=address -o leaks c/main.c
./leaks < tests/01-basic.in
```

```text
==6297==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 50 byte(s) in 3 object(s) allocated from:
    #0 0x7f69ff8fd9c7 in malloc ../../../../src/libsanitizer/asan/asan_malloc_linux.cpp:69
    #1 0x55a2bc4f3444 in counted_malloc c/main.c:15
    #2 0x55a2bc4f37e2 in describe c/main.c:76
    #3 0x55a2bc4f3bec in show c/main.c:124
    #4 0x55a2bc4f41fc in main c/main.c:175
...
SUMMARY: AddressSanitizer: 163 byte(s) leaked in 12 allocation(s).
```

The report comes after the program's own output. Read each stack from the top down,
skipping `malloc` and `counted_malloc`. This one says that three blocks allocated by
`describe`, called from `show`, were never freed.

**Valgrind** runs the program as it is, without rebuilding for it, and watches every
allocation. It is slower, and it finds the same leaks:

```sh
cc -std=c17 -g -O0 -o leaks c/main.c
valgrind --leak-check=full ./leaks < tests/01-basic.in
```

```text
==6227== 113 (24 direct, 89 indirect) bytes in 1 blocks are definitely lost in loss record 5 of 5
==6227==    at 0x4846828: malloc (in /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so)
==6227==    by 0x1092E4: counted_malloc (main.c:15)
==6227==    by 0x1093DB: song_new (main.c:49)
==6227==    by 0x109566: add (main.c:82)
==6227==    by 0x109989: main (main.c:163)
==6227==
==6227== LEAK SUMMARY:
==6227==    definitely lost: 74 bytes in 4 blocks
==6227==    indirectly lost: 89 bytes in 8 blocks
```

A block is **definitely lost** when nothing points to it any more, and **indirectly
lost** when the only pointers to it are inside other lost blocks. Here one song is
lost, and with it everything it points to. Fix the definitely lost blocks first; the
indirect ones often go with them. Build with `-g` so the reports have line numbers, and
run each test, because each one goes down different paths.

**Things to notice:**

- **Every allocation needs an owner that frees it.** For each `counted_malloc`, ask
  which code frees that block, on every path. The five leaks are five different ways
  to get the answer wrong: an early return, a struct freed without the strings it
  owns, a pointer overwritten, a returned string nobody frees, and a list dropped
  without freeing it.
- **Setting a pointer to `NULL` frees nothing.** In Python or Java, `items = []` lets
  the old list be collected. In C, it only loses the last pointer to the old memory.
- **Freeing a list takes a loop that is easy to get wrong.** `for (s = first; s; s =
  s->next) free(s);` reads `s->next` after `s` was freed. That is a use after free, it
  often seems to work, and AddressSanitizer stops the program with
  `heap-use-after-free`. Save `next` before you free.
- **A fix can add a bug.** Free something too early, or twice, and the tools report
  that too. Run them again after each fix.

**Stretch:** add `undo`, which brings back the last song that `remove` or `clear`
took away. You have to keep removed songs alive instead of freeing them, and still
free them all when the program ends.
