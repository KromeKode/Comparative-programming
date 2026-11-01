# Exercise 10.3: Folders that know their parent

This exercise is in C, C++, and Rust. Your program keeps a tree of folders. The root is
called `/` and always exists. Every other folder has a name that is unique in the whole
tree, so a command can name any folder without a path. Each line of input is a command:

| Command | Effect |
| --- | --- |
| `mkdir NAME PARENT` | Make a folder called NAME at the end of PARENT's list, and print `made PATH` |
| `path NAME` | Print the folder's path, such as `/docs/notes` |
| `ls NAME` | Print `NAME: CHILD CHILD ...` in the order they were added, or `NAME: (empty)` |
| `rm NAME` | Remove the folder and everything in it, and print `removed NAME, freed N folders` |
| `mv NAME PARENT` | Move the folder and everything in it to the end of PARENT's list, and print `moved NAME to PATH` |

A path is `/` for the root, and otherwise `/` followed by the names from just below the
root down to the folder, joined by `/`. Find it by following parent links up from the
folder. When the input ends, free the whole tree and print `freed N folders at exit`,
which counts the root too. Write `1 folder` for one and `N folders` for any other count.

**Count the folders where they are freed:** in the destructor in C++, in `Drop` in
Rust, and in the function that frees a folder in C. Don't count the folders you meant
to free. Then the numbers show what your program really freed, and a leak shows up as
a number that is too small.

If a command is wrong, print one of these errors and change nothing. Check the names
from left to right.

| Problem | Error |
| --- | --- |
| Not one of the five commands | `error: unknown command "cd"` |
| Too few or too many names | `error: mkdir takes 2 names, got 1` or `error: ls takes 1 name, got 2` |
| A name that is not in the tree | `error: no folder "ghost"` |
| `mkdir` with a name that is taken | `error: "docs" already exists` |
| `rm /` | `error: cannot remove /` |
| `mv /` | `error: cannot move /` |
| `mv` into the folder itself or anything inside it | `error: cannot move docs into itself` |

For `mkdir`, check that NAME is free before checking PARENT. For `mv`, check NAME,
then that NAME is not `/`, then PARENT. Moving a folder to the parent it is already in
puts it at the end of the list. Skip blank lines, and allow any number of spaces
between the parts of a line. Names are under 32 ASCII characters, and the tests have
at most 200 folders.

| Input | Output |
| --- | --- |
| `mkdir docs /` | `made /docs` |
| `mkdir notes docs` | `made /docs/notes` |
| `mkdir music /` | `made /music` |
| `mv notes music` | `moved notes to /music/notes` |
| `ls /` | `/: docs music` |
| `rm music` | `removed music, freed 2 folders` |
| | `freed 2 folders at exit` |

```sh
python3 tools/check.py 10 --lang rust
```

**Things to notice:**

- **A parent owns its children, and a child must not own its parent.** In Rust, keep
  the children as `Vec<Rc<RefCell<Folder>>>` and the parent as
  `Weak<RefCell<Folder>>`. Make the parent an `Rc` instead and every parent and child
  own each other: the program still prints the right paths, but every `rm` says
  `freed 0 folders`, and so does the end.
- **C++ needs no shared pointers at all.** Each folder can own its children as
  `std::vector<std::unique_ptr<Folder>>` and keep a plain `Folder *` to its parent,
  which owns nothing. The parent always outlives its children, so that pointer never
  dangles. Removing a child from the vector destroys it and, through its own vector,
  everything inside it.
- **In C, the same ownership lives only in comments.** Free a folder's children before
  the folder, and take a folder out of its parent's list before you free it, or the
  list points at freed memory.
- **`mv` takes ownership away from one parent and gives it to another.** In C++ that is
  a `std::move` of the `unique_ptr` out of one vector and into the other. In Rust it
  is removing an `Rc` from one `Vec` and pushing it onto another, then pointing the
  `Weak` at the new parent.
- **Following parent links is how you stop a cycle.** Moving `docs` into
  `docs/notes` would make the folders own each other in a loop that nothing can free.
  Walk up from the new parent: if you meet the folder you are moving, refuse.

**Stretch:** add `cp NAME PARENT`, which copies a folder and everything in it. The
copies need new, unique names: append `-copy`, then `-copy2`, `-copy3`, and so on,
to each name that is taken. Which language makes a deep copy of this tree easiest,
and why?
