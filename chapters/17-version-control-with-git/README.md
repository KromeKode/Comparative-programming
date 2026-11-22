# Chapter 17: Version control with Git

Every chapter so far has been about one program at a time. Real programs change for
years, and several people change them at once. Version control keeps every version of
every file, says who changed what and why, lets you try an idea on a branch and throw
it away, and merges the work of people who never sat in the same room. Git is the
version control system almost everyone uses now, from the Linux kernel to this book,
whose chapters each arrived as a branch and a pull request. This chapter uses no
programming language. Its examples are terminal sessions, and its exercises are
scripts of Git commands.

**Languages:** none; Git 2.43 and bash · **Time:** about 2½ hours

| You will learn | Why it matters later |
| --- | --- |
| Commits, the staging area, and what a repository holds | Every chapter after this one, which you can now keep in a repository of your own |
| Branches, merging, and resolving a conflict | Modules, packages, builds (chapter 18), design problems (chapter 31) |
| Undoing and rewriting: restore, reset, revert, rebase, and the reflog | Debugging and profiling (chapter 20) |
| Remotes, pushing, pulling, and pull requests | Testing basics (chapter 19), where tests run on every push |
| Searching history with `log`, `blame`, and `bisect` | Debugging and profiling (chapter 20) |

---

## 17.1 The concept: snapshots in a graph

A **repository** is a folder whose history Git keeps, in a hidden `.git` folder inside
it. Each time you **commit**, Git saves a snapshot of every file in the project, with
your name, the time, a message saying why, and a link to the commit before it, its
**parent**. A commit is never changed afterward, and it is named by a hash of everything
in it, a 40-digit hexadecimal number such as `ce01362...`, which you can shorten to its
first seven or so digits.

Following the parent links from the newest commit back to the first gives the history.
When two people start from the same commit and each commits something different, the
history forks, and a **merge commit** with two parents joins it up again. So the history
is a graph, with the arrows pointing back in time:

```text
A <-- B <-- C <-- F       main
       \         /
        D <---- E         feature
```

A **branch** is a name for one commit, the newest on that line of work. Committing on a
branch moves its name to the new commit. Here `main` names F and `feature` names E, and
deleting `feature` would delete only the name: E is still in the history, because F
points at it. **HEAD** is Git's name for where you are, usually a branch, so that the
next commit knows its parent and which name to move.

Git is **distributed**. A copy of a repository, a **clone**, has the whole history, so
you can commit, branch, look at old versions, and search the history with no network at
all. A server such as GitHub holds one more copy, which everyone agrees to share work
through. Sending your commits to it is a **push**, and bringing other people's commits
from it is a **fetch**.

Between your files and the history sits the **index**, also called the staging area.
`git add` copies a file into it, and `git commit` makes the snapshot from it, not from
your files. That lets you commit some of your changes and keep working on the rest, so
each commit can be one change that makes sense on its own.

| Place | What it is | What puts things there |
| --- | --- | --- |
| Working tree | The files you edit, in the project folder | Your editor, and `git switch` and `git restore` |
| Index | The snapshot that the next commit will be | `git add`, `git rm`, and `git restore --staged` |
| Repository | Every commit, in `.git` | `git commit`, `git merge`, and `git fetch` |

---

## 17.2 Your first repository

Git needs your name and email once, to put in every commit you make, and a name for the
first branch of a new repository. These go in `~/.gitconfig`, for every repository you
use:

```sh
git config --global user.name "Ada Lovelace"
git config --global user.email ada@example.com
git config --global init.defaultBranch main
```

Every session in this chapter ran in a fresh home folder with those settings, through
the same script that runs the exercises, `tools/git-sandbox.sh`. It also starts Git's
clock at 9:00 on 1 March 2026 and moves it on a minute for each command, so the hashes
you see are the hashes that the script makes on any machine. Running the commands
yourself gives the same output, apart from the hashes, the times, and the folder
names.

`git init` makes a repository, and `git status` says what Git thinks of each file. A
file that Git has never been told about is **untracked**:

```console
$ git init notes
Initialized empty Git repository in /home/ada/notes/.git/
$ cd notes
$ printf 'milk\neggs\n' > shopping.txt
$ git status
On branch main

No commits yet

Untracked files:
  (use "git add <file>..." to include in what will be committed)
	shopping.txt

nothing added to commit but untracked files present (use "git add" to track)
```

`git add` copies the file into the index, and `git commit` makes a commit from the
index. `-m` gives the message; without it, Git opens an editor to write one. The first
commit is the **root commit**, the only one with no parent:

```console
$ git add shopping.txt
$ git status
On branch main

No commits yet

Changes to be committed:
  (use "git rm --cached <file>..." to unstage)
	new file:   shopping.txt

$ git commit -m "Start a shopping list"
[main (root-commit) 21218f5] Start a shopping list
 1 file changed, 2 insertions(+)
 create mode 100644 shopping.txt
```

Now change the file. `git status --short` gives two columns: the first compares the
index with the last commit, and the second compares the working tree with the index. So
` M` is a change you haven't added, and `M ` is one you have. `git diff` shows the
changes you haven't added yet, and `git diff --staged` the ones that the next commit
will have. A diff marks the lines it adds with `+` and the lines it removes with `-`,
with a few unchanged lines around them for context:

```console
$ printf 'bread\n' >> shopping.txt
$ git status --short
 M shopping.txt
$ git diff
diff --git a/shopping.txt b/shopping.txt
index d64b1a1..01ec3d2 100644
--- a/shopping.txt
+++ b/shopping.txt
@@ -1,2 +1,3 @@
 milk
 eggs
+bread
$ git add shopping.txt
$ git status --short
M  shopping.txt
$ git diff --staged
diff --git a/shopping.txt b/shopping.txt
index d64b1a1..01ec3d2 100644
--- a/shopping.txt
+++ b/shopping.txt
@@ -1,2 +1,3 @@
 milk
 eggs
+bread
$ git commit -m "Add bread"
[main e0c0efe] Add bread
 1 file changed, 1 insertion(+)
```

`git log` lists the commits, newest first, and `--oneline` gives each one a line, with
its hash shortened:

```console
$ git log
commit e0c0efedc67fba25770768e3b2cd04377d8888ef
Author: Ada Lovelace <ada@example.com>
Date:   Sun Mar 1 09:16:00 2026 +0000

    Add bread

commit 21218f5dcb92c3622b9ac3c64fcd33511039d815
Author: Ada Lovelace <ada@example.com>
Date:   Sun Mar 1 09:09:00 2026 +0000

    Start a shopping list
$ git log --oneline
e0c0efe Add bread
21218f5 Start a shopping list
```

A commit message starts with a **subject** line, which `--oneline` shows. Keep it under
about 50 characters, and write it as a command, "Add bread" rather than "Added bread",
so that it reads as what the commit does when it is applied. For more, leave a blank
line and write a body that says why, since the diff already says what. `git commit` with
no `-m` opens an editor for exactly that.

---

## 17.3 What's inside `.git`

Git's history is a small database of **objects**, each stored under the hash of its
contents. A file's contents are a **blob**. `git hash-object` gives the hash that a file
would have, and the hash is the SHA-1 of the word `blob`, the size, a zero byte, and
the contents, so `sha1sum` gets the same answer. `git add` writes the blob into
`.git/objects`, in a folder named after the first two digits, and `git cat-file` reads
it back: `-t` for its type, `-p` to print it.

```console
$ git init -q notes
$ cd notes
$ echo hello > greeting.txt
$ git hash-object greeting.txt
ce013625030ba8dba906f756967f9e9ca394464a
$ echo hello | git hash-object --stdin
ce013625030ba8dba906f756967f9e9ca394464a
$ printf 'blob 6\0hello\n' | sha1sum
ce013625030ba8dba906f756967f9e9ca394464a  -
$ git add greeting.txt
$ find .git/objects -type f
.git/objects/ce/013625030ba8dba906f756967f9e9ca394464a
$ git cat-file -t ce01362
blob
$ git cat-file -p ce01362
hello
```

A commit is an object too. It holds the hash of a **tree**, the parent's hash (the root
commit has none), the author and committer with their times in seconds since 1970, and
the message. A tree is a list of names, each with a mode, a type, and the hash of a
blob or of another tree for a folder. A branch is a file holding a commit's hash, and
`HEAD` is a file that names the branch you are on:

```console
$ git commit -q -m "Say hello"
$ git cat-file -p HEAD
tree 57e9529754dc514a3ec10db2ff882018fbe1fcbf
author Ada Lovelace <ada@example.com> 1772356380 +0000
committer Ada Lovelace <ada@example.com> 1772356380 +0000

Say hello
$ git cat-file -p 'HEAD^{tree}'
100644 blob ce013625030ba8dba906f756967f9e9ca394464a	greeting.txt
$ cat .git/HEAD
ref: refs/heads/main
$ cat .git/refs/heads/main
4d5a9c74907023dcc35703ab906bd53b909842d7
```

Naming things by their contents has consequences that the rest of Git depends on:

- **The same contents are stored once.** Below, `copy.txt` has the same contents as
  `greeting.txt`, so both names in the tree point at blob `ce01362`, and the second
  commit adds four objects, not five: a blob for `about.txt`, a tree for `docs`, a new
  top tree, and the commit itself. Each commit is a full snapshot, but a file that
  didn't change costs nothing.
- **Nothing can change unnoticed.** Changing one byte of an old file would change its
  blob's hash, so the tree that holds it, and the commit that holds that, and every
  commit after it. A commit's hash vouches for the whole history behind it, and two
  clones with the same hash for `main` have the same history, byte for byte.
- **Commits can't be edited.** "Changing" a commit, as `--amend` and `rebase` do, makes
  a new commit with a new hash, and moves the branch to it. The old one is still in the
  database until Git cleans up.

```console
$ cp greeting.txt copy.txt
$ mkdir docs
$ echo "Notes about notes." > docs/about.txt
$ git add .
$ git commit -q -m "Add a copy and a folder"
$ git cat-file -p 'HEAD^{tree}'
100644 blob ce013625030ba8dba906f756967f9e9ca394464a	copy.txt
040000 tree 552645e24738e6efce7c8d40448d8b40d18faf66	docs
100644 blob ce013625030ba8dba906f756967f9e9ca394464a	greeting.txt
$ git cat-file -p HEAD
tree 7c5592fb4a7a976b7a011dc7f557058d1ccae262
parent 4d5a9c74907023dcc35703ab906bd53b909842d7
author Ada Lovelace <ada@example.com> 1772356920 +0000
committer Ada Lovelace <ada@example.com> 1772356920 +0000

Add a copy and a folder
$ find .git/objects -type f | sort
.git/objects/4d/5a9c74907023dcc35703ab906bd53b909842d7
.git/objects/55/2645e24738e6efce7c8d40448d8b40d18faf66
.git/objects/57/e9529754dc514a3ec10db2ff882018fbe1fcbf
.git/objects/7c/5592fb4a7a976b7a011dc7f557058d1ccae262
.git/objects/83/a89385bf401268f45295f9bc3c83d3d9be2b1c
.git/objects/9a/68ce438475cf6a547c052f3705e270f501270c
.git/objects/ce/013625030ba8dba906f756967f9e9ca394464a
```

Seven objects: two blobs, three trees, and two commits. A repository with years of
history doesn't keep every version of every file whole: `git gc`, which Git runs by
itself from time to time, packs the objects into a **packfile** and stores most of them
as differences from similar objects. Git has used SHA-1 since 2005, and a repository can
use SHA-256 instead, with `git init --object-format=sha256`. In Git 2.43 such a
repository can't exchange commits with a SHA-1 one, so hosting services have been slow
to take it up.

---

## 17.4 Branches and merging

`git branch NAME` makes a branch on the commit you are on, `git branch` lists them with
a `*` on yours, and `git switch NAME` moves you to one, changing your files to match.
`git log --graph --all` draws every branch:

```console
$ git init -q cafe
$ cd cafe
$ printf 'Soup 4\nSalad 5\nBread 2\n' > menu.txt
$ git add menu.txt
$ git commit -q -m "Start the menu"
$ git branch pie
$ git branch
* main
  pie
$ git switch pie
Switched to branch 'pie'
$ printf 'Pie 3\n' >> menu.txt
$ git commit -q -a -m "Add pie"
$ git log --oneline --graph --all
* 8ecbc01 Add pie
* d59cdd3 Start the menu
```

Merging `pie` into `main` needs no new commit, because `main` hasn't moved since `pie`
started from it. Git moves the `main` label up to `pie`'s commit and says
**Fast-forward**:

```console
$ git switch main
Switched to branch 'main'
$ git merge pie
Updating d59cdd3..8ecbc01
Fast-forward
 menu.txt | 1 +
 1 file changed, 1 insertion(+)
$ git log --oneline --graph --all
* 8ecbc01 Add pie
* d59cdd3 Start the menu
```

When both branches have moved, Git makes a merge commit. It finds the **merge base**,
the newest commit that both branches have, and works out what each branch changed since
then. Changes to different files, or to different parts of one file, are combined. This
is a **three-way merge**, and "ort" is the name of the algorithm that does it, Git's
default since 2.34. `git switch -c` makes a branch and switches to it, and `-a` on a
commit adds every file that Git already tracks:

```console
$ git switch -c drinks
Switched to a new branch 'drinks'
$ printf 'Tea 1\nCoffee 2\n' > drinks.txt
$ git add drinks.txt
$ git commit -q -m "Add a drinks list"
$ git switch main
Switched to branch 'main'
$ printf 'Soup 4\nSalad 5\nBread 3\nPie 3\n' > menu.txt
$ git commit -q -a -m "Charge more for bread"
$ git log --oneline --graph --all
* b8edaee Charge more for bread
| * 6f35c6e Add a drinks list
|/
* 8ecbc01 Add pie
* d59cdd3 Start the menu
$ git merge -m "Merge the drinks list" drinks
Merge made by the 'ort' strategy.
 drinks.txt | 2 ++
 1 file changed, 2 insertions(+)
 create mode 100644 drinks.txt
$ git log --oneline --graph
*   a4a53e7 Merge the drinks list
|\
| * 6f35c6e Add a drinks list
* | b8edaee Charge more for bread
|/
* 8ecbc01 Add pie
* d59cdd3 Start the menu
$ git branch -d pie drinks
Deleted branch pie (was 8ecbc01).
Deleted branch drinks (was 6f35c6e).
```

`git branch -d` deletes a branch once its commits are merged, and refuses otherwise;
`-D` deletes it anyway.

When both branches change the same lines, Git can't choose, and that is a **conflict**.
The merge stops halfway, with the files that merged cleanly already in the index and the
conflicted file in the working tree, marked up. Between `<<<<<<<` and `=======` is your
side, "ours", the branch you are on; between `=======` and `>>>>>>>` is "theirs", the
branch you are merging in. The `zdiff3` style also shows the merge base, after
`|||||||`, which often makes it clear what each side meant to do.
`git checkout --conflict=zdiff3` redraws a conflict that way, and
`git config --global merge.conflictStyle zdiff3` makes it the default:

```console
$ git switch -c cheaper
Switched to a new branch 'cheaper'
$ printf 'Soup 3\nSalad 5\nBread 3\nPie 3\n' > menu.txt
$ git commit -q -a -m "Make soup cheaper"
$ git switch main
Switched to branch 'main'
$ printf 'Soup 5\nSalad 5\nBread 3\nPie 3\n' > menu.txt
$ git commit -q -a -m "Make soup dearer"
$ git merge cheaper
Auto-merging menu.txt
CONFLICT (content): Merge conflict in menu.txt
Automatic merge failed; fix conflicts and then commit the result.
$ git status
On branch main
You have unmerged paths.
  (fix conflicts and run "git commit")
  (use "git merge --abort" to abort the merge)

Unmerged paths:
  (use "git add <file>..." to mark resolution)
	both modified:   menu.txt

no changes added to commit (use "git add" and/or "git commit -a")
$ cat menu.txt
<<<<<<< HEAD
Soup 5
=======
Soup 3
>>>>>>> cheaper
Salad 5
Bread 3
Pie 3
$ git checkout --conflict=zdiff3 menu.txt
Recreated 1 merge conflict
$ cat menu.txt
<<<<<<< ours
Soup 5
||||||| base
Soup 4
=======
Soup 3
>>>>>>> theirs
Salad 5
Bread 3
Pie 3
```

To resolve the conflict, edit the file to what it should say, remove the markers, add
it, and commit. Here the two changes cancel out and soup stays at 4. `--no-edit` takes
the message Git has prepared. If you would rather not merge after all,
`git merge --abort` puts everything back as it was:

```console
$ printf 'Soup 4\nSalad 5\nBread 3\nPie 3\n' > menu.txt
$ git add menu.txt
$ git status --short
M  menu.txt
$ git commit -q --no-edit
$ git log --oneline --graph
*   6b33629 Merge branch 'cheaper'
|\
| * 9a05456 Make soup cheaper
* | 995efce Make soup dearer
|/
*   a4a53e7 Merge the drinks list
|\
| * 6f35c6e Add a drinks list
* | b8edaee Charge more for bread
|/
* 8ecbc01 Add pie
* d59cdd3 Start the menu
```

---

## 17.5 Undoing things

Each mistake has its own way back, depending on how far the change has got: into the
working tree, into the index, into a commit, or into a commit that other people have.
The sessions in this section and the next start from a repository that a setup script
made first:

```sh
# A repository with three commits, to undo things in.
git init -q cafe
cd cafe
printf 'Soup 4\nSalad 5\n' > menu.txt
git add menu.txt
tick; git commit -q -m "Start the menu"
printf 'Ada\n' > staff.txt
git add staff.txt
tick; git commit -q -m "Add the staff list"
printf 'Soup 4\nSalad 5\nPie 3\n' > menu.txt
tick; git commit -q -a -m "Add pie"
```

`git restore FILE` throws away the changes to a file in the working tree, putting back
the version in the index. `git restore --staged FILE` takes a file out of the index,
putting back the version in the last commit, and leaves your working tree alone:

```console
$ cd cafe
$ git log --oneline
a54f302 Add pie
004e33b Add the staff list
d9732ee Start the menu
$ printf 'Soup 40\nSalad 5\nPie 3\n' > menu.txt
$ git status --short
 M menu.txt
$ git restore menu.txt
$ git status --short
$ printf 'Soup 4\nSalad 5\nPie 3\nCake 4\n' > menu.txt
$ git add menu.txt
$ git status --short
M  menu.txt
$ git restore --staged menu.txt
$ git status --short
 M menu.txt
```

`git commit --amend` replaces the last commit with a new one, here to fix a typo in the
message. Any changes in the index go into the new commit as well:

```console
$ git commit -q -a -m "Add cak"
$ git log --oneline -1
5e461b1 Add cak
$ git commit -q --amend -m "Add cake"
$ git log --oneline
d85a2e9 Add cake
a54f302 Add pie
004e33b Add the staff list
d9732ee Start the menu
```

`git reset` moves the branch you are on to another commit. `HEAD~1` is the commit before
`HEAD`, and `HEAD~3` the one three back. What happens to the commits that the branch no
longer reaches depends on the mode:

- `--soft` moves only the branch, so the changes of the commits it skipped are still in
  the index, ready to commit again, as one commit if you like.
- The default, `--mixed`, also resets the index, so the changes are only in the working
  tree.
- `--hard` resets the working tree too, and throws away every uncommitted change.

```console
$ git reset --soft HEAD~1
$ git status --short
M  menu.txt
$ git log --oneline -1
a54f302 Add pie
$ git reset HEAD~1
Unstaged changes after reset:
M	menu.txt
$ git status --short
 M menu.txt
$ git log --oneline -1
004e33b Add the staff list
```

Those three resets left three commits behind, and the last one threw away the
uncommitted changes too. The commits aren't gone: the **reflog** lists every commit that
`HEAD` has been on, for the last 90 days by default, and `HEAD@{3}` means "where HEAD
was three moves ago". Resetting to it brings everything back. What the reflog can't
bring back is a change that was never committed: `--hard` threw away the working tree's
copy of `Cake 4`, and if it hadn't also been in a commit, it would be gone.

```console
$ git reset --hard HEAD~1
HEAD is now at d9732ee Start the menu
$ git status --short
$ cat menu.txt
Soup 4
Salad 5
$ git log --oneline
d9732ee Start the menu
$ git reflog
d9732ee HEAD@{0}: reset: moving to HEAD~1
004e33b HEAD@{1}: reset: moving to HEAD~1
a54f302 HEAD@{2}: reset: moving to HEAD~1
d85a2e9 HEAD@{3}: commit (amend): Add cake
5e461b1 HEAD@{4}: commit: Add cak
a54f302 HEAD@{5}: commit: Add pie
004e33b HEAD@{6}: commit: Add the staff list
d9732ee HEAD@{7}: commit (initial): Start the menu
$ git reset --hard HEAD@{3}
HEAD is now at d85a2e9 Add cake
$ git log --oneline
d85a2e9 Add cake
a54f302 Add pie
004e33b Add the staff list
d9732ee Start the menu
```

All of those rewrite history, which is fine for commits that nobody else has. For a
commit that others already have, `git revert` makes a new commit that undoes it, and
the history keeps both:

```console
$ git revert --no-edit HEAD~2
[main cef68c4] Revert "Add the staff list"
 Date: Sun Mar 1 09:34:00 2026 +0000
 1 file changed, 1 deletion(-)
 delete mode 100644 staff.txt
$ git log --oneline
cef68c4 Revert "Add the staff list"
d85a2e9 Add cake
a54f302 Add pie
004e33b Add the staff list
d9732ee Start the menu
$ ls
menu.txt
```

| You want to | Command | Gone for good if you're wrong |
| --- | --- | --- |
| Throw away changes to a file | `git restore FILE` | The changes, which were never committed |
| Take a file out of the next commit | `git restore --staged FILE` | Nothing |
| Fix the last commit or its message | `git commit --amend` | Nothing; the old one is in the reflog |
| Undo commits, keeping their changes | `git reset --soft COMMIT` or `git reset COMMIT` | Nothing |
| Undo commits and every uncommitted change | `git reset --hard COMMIT` | Every uncommitted change |
| Undo a commit that others have | `git revert COMMIT` | Nothing; it adds a commit |
| Get back a commit you reset away | `git reflog`, then `git reset --hard HEAD@{N}` | Nothing |

---

## 17.6 Rewriting history: rebase, cherry-pick, and stash

A **rebase** replays a branch's commits on top of another commit, as though the branch
had started there. Here `soups` started before `main` moved on, and rebasing it onto
`main` makes new copies of its two commits, with new hashes, after `main`'s newest
commit. The history becomes a straight line, and merging is a fast-forward:

```console
$ cd cafe
$ git log --oneline --graph --all
* 4993811 Add opening hours
* 9fb60aa Add the staff list
| * da23edc Add tomato soup
| * 62db526 Add leek soup
|/
* d9732ee Start the menu
$ git switch soups
Switched to branch 'soups'
$ git rebase main
Successfully rebased and updated refs/heads/soups.
$ git log --oneline --graph --all
* d8e4707 Add tomato soup
* f911f34 Add leek soup
* 4993811 Add opening hours
* 9fb60aa Add the staff list
* d9732ee Start the menu
```

```console
$ git switch main
Switched to branch 'main'
$ git merge soups
Updating 4993811..d8e4707
Fast-forward
 leek.txt   | 1 +
 tomato.txt | 1 +
 2 files changed, 2 insertions(+)
 create mode 100644 leek.txt
 create mode 100644 tomato.txt
$ git branch -d soups
Deleted branch soups (was d8e4707).
```

Merging and rebasing end with the same files. They differ in the history they leave: a
merge keeps what really happened, two lines of work joined, and a rebase tells a tidier
story, as though the work had been done in a straight line. Many teams rebase their own
branches before merging, so that `main` reads as a list of finished changes.

`git cherry-pick` copies one commit onto the branch you are on. The copy has a new hash,
and keeps the original's author and the date it was written:

```console
$ git switch -c winter
Switched to a new branch 'winter'
$ printf 'Open 10 to 4\n' > hours.txt
$ git commit -q -a -m "Open later in winter"
$ printf 'Soup 5\nSalad 5\n' > menu.txt
$ git commit -q -a -m "Charge more for soup"
$ git switch main
Switched to branch 'main'
$ git cherry-pick winter
[main 1c4624f] Charge more for soup
 Date: Sun Mar 1 09:20:00 2026 +0000
 1 file changed, 1 insertion(+), 1 deletion(-)
$ git log --oneline --graph --all
* 1c4624f Charge more for soup
| * b6377f6 Charge more for soup
| * c7437ac Open later in winter
|/
* d8e4707 Add tomato soup
* f911f34 Add leek soup
* 4993811 Add opening hours
* 9fb60aa Add the staff list
* d9732ee Start the menu
```

`git stash` puts your uncommitted changes aside, in a commit that no branch points at,
and leaves a clean working tree. `git stash pop` brings them back and drops the entry:

```console
$ printf 'Ada\nGrace\n' > staff.txt
$ git status --short
 M staff.txt
$ git stash
Saved working directory and index state WIP on main: 1c4624f Charge more for soup
$ git status --short
$ git stash list
stash@{0}: WIP on main: 1c4624f Charge more for soup
$ git stash pop
On branch main
Changes not staged for commit:
  (use "git add <file>..." to update what will be committed)
  (use "git restore <file>..." to discard changes in working directory)
	modified:   staff.txt

no changes added to commit (use "git add" and/or "git commit -a")
Dropped refs/stash@{0} (0b6d40b7f302261af4b71930a39279c18e4d76f5)
$ git status --short
 M staff.txt
```

An **interactive rebase** lets you edit the list of commits before they are replayed:
reorder the lines, change `pick` to `reword` to change a message, to `squash` or `fixup`
to fold a commit into the one before it, or to `drop` to leave it out. Git opens the
list in your editor. A commit made with `git commit --fixup COMMIT` is called
`fixup! SUBJECT`, and `--autosquash` moves it under the commit it fixes and marks it
`fixup` for you. Below, the "editor" is a `sed` command that prints the list, without
the comment lines that explain the commands, and leaves it as it is, so you can see what
Git suggested. Newer versions of Git write each line as
`pick d4270bd # Open an hour later`, with the subject after a `#` to show that only the
hash counts, and `s/ # / /` takes the `#` out so that the list prints the same in every
version:

```console
$ git restore staff.txt
$ printf 'Open 9 to 6\n' > hours.txt
$ git commit -q -a -m "Open an hour later"
$ printf 'Grace\n' > cook.txt
$ git add cook.txt
$ git commit -q -m "Name the cook"
$ printf 'Open 9 to 7\n' > hours.txt
$ git commit -q -a --fixup HEAD~1
$ git log --oneline -4
18f6041 fixup! Open an hour later
2b62596 Name the cook
d4270bd Open an hour later
1c4624f Charge more for soup
$ GIT_SEQUENCE_EDITOR="sed -n 's/ # / /; /^[^#]/p'" git rebase -i --autosquash HEAD~3
pick d4270bd Open an hour later
fixup 18f6041 fixup! Open an hour later
pick 2b62596 Name the cook
Successfully rebased and updated refs/heads/main.
$ git log --oneline -3
c47ae0c Name the cook
85296ab Open an hour later
1c4624f Charge more for soup
```

Rebasing and amending make new commits in place of old ones, and resetting moves a
branch off its commits, so the rule is: **never rewrite commits that other people
already have.** If you push a branch,
rewrite it, and push again, Git refuses, because the server's branch has commits that
yours doesn't. Forcing it with `git push --force` replaces the server's branch, and
anyone who had built on the old commits now has work based on commits that no longer
exist. On your own branch, before anyone else uses it, rewrite as much as you like, and
push with `--force-with-lease`, which refuses if someone else pushed in the meantime.

---

## 17.7 Working with others

A **remote** is another copy of the repository that yours knows by a name. `git clone`
copies a repository and remembers where it came from as `origin`. Here a bare
repository, one with no working tree, stands in for a server like GitHub.
`git push -u origin main` sends `main` there, and `-u` makes `origin/main` the
**upstream** of your `main`, so that plain `git push` and `git pull` know where to go
from then on. `origin/main` is a **remote-tracking branch**: your repository's note of
where `main` was on `origin` when you last talked to it.

```console
$ git init -q --bare cafe.git
$ git clone cafe.git ada
Cloning into 'ada'...
warning: You appear to have cloned an empty repository.
done.
$ cd ada
$ printf 'Soup 4\nSalad 5\n' > menu.txt
$ git add menu.txt
$ git commit -q -m "Start the menu"
$ git push -u origin main
To /home/ada/cafe.git
 * [new branch]      main -> main
branch 'main' set up to track 'origin/main'.
$ git branch -vv
* main d655f6e [origin/main] Start the menu
```

Grace clones the same repository, commits, and pushes. Each clone has its own settings,
in `.git/config`, so she sets her own name there without `--global`:

```console
$ cd ..
$ git clone -q cafe.git grace
$ cd grace
$ git config user.name "Grace Hopper"
$ git config user.email grace@example.com
$ printf 'Grace\n' > staff.txt
$ git add staff.txt
$ git commit -q -m "Add the staff list"
$ git push -q
```

Now Ada's push is rejected. The server's `main` has Grace's commit, which Ada doesn't
have, and accepting the push would throw it away. `git fetch` brings Grace's commit into
Ada's repository and moves `origin/main`, without touching Ada's branch or files, and
`git status` compares the two:

```console
$ cd ../ada
$ printf 'Soup 4\nSalad 5\nPie 3\n' > menu.txt
$ git commit -q -a -m "Add pie"
$ git push
To /home/ada/cafe.git
 ! [rejected]        main -> main (fetch first)
error: failed to push some refs to '/home/ada/cafe.git'
hint: Updates were rejected because the remote contains work that you do not
hint: have locally. This is usually caused by another repository pushing to
hint: the same ref. If you want to integrate the remote changes, use
hint: 'git pull' before pushing again.
hint: See the 'Note about fast-forwards' in 'git push --help' for details.
$ git fetch
From /home/ada/cafe
   d655f6e..f753490  main       -> origin/main
$ git status
On branch main
Your branch and 'origin/main' have diverged,
and have 1 and 1 different commits each, respectively.
  (use "git pull" if you want to integrate the remote branch with yours)

nothing to commit, working tree clean
$ git log --oneline --graph --all
* 0b07296 Add pie
| * f753490 Add the staff list
|/
* d655f6e Start the menu
```

`git pull` is a fetch followed by a merge or a rebase onto the upstream branch. When
only one side has new commits, either one is a fast-forward. When both sides do, as
here, Git won't guess which you want, and stops before changing anything. `--rebase`
says to rebase, which is safe here, because Ada's commit hasn't been pushed, and it
keeps the history a straight line instead of adding a merge commit for every pull.
`git config --global pull.rebase true` makes it the default, and `pull.rebase false`
makes it merge:

```console
$ git pull
hint: You have divergent branches and need to specify how to reconcile them.
hint: You can do so by running one of the following commands sometime before
hint: your next pull:
hint:
hint:   git config pull.rebase false  # merge
hint:   git config pull.rebase true   # rebase
hint:   git config pull.ff only       # fast-forward only
hint:
hint: You can replace "git config" with "git config --global" to set a default
hint: preference for all repositories. You can also pass --rebase, --no-rebase,
hint: or --ff-only on the command line to override the configured default per
hint: invocation.
fatal: Need to specify how to reconcile divergent branches.
$ git pull --rebase
Successfully rebased and updated refs/heads/main.
$ git log --oneline --graph --all
* d8a523d Add pie
* f753490 Add the staff list
* d655f6e Start the menu
$ git push
To /home/ada/cafe.git
   f753490..d8a523d  main -> main
$ git log --format='%h %an: %s'
d8a523d Ada Lovelace: Add pie
f753490 Grace Hopper: Add the staff list
d655f6e Ada Lovelace: Start the menu
```

On GitHub and GitLab, changes usually arrive through a **pull request** (GitLab says
**merge request**): you push a branch, and ask for it to be merged into `main`. Others
review the diff and comment on lines of it, you push more commits to the branch to
answer them, and automated checks run on every push. Each chapter of this book arrived
that way, and its checks run `python3 tools/check.py all --solutions` on every one. When
you can't push to a repository, you **fork** it, a copy on the server under your own
name, push your branch there, and open the pull request from the fork.

---

## 17.8 Searching history

`git log -- FILE` lists only the commits that changed that file, and `-S TEXT`, called
the pickaxe, lists the commits that changed how many times some text appears, which
finds the commit that added a line, or the one that removed it. `-p` shows each
commit's diff:

```sh
# A cafe with a short history, and a test that checks every price is from 1 to 20.
git init -q cafe
cd cafe
save() { tick; git add --all; git commit -q -m "$1"; }
cat > check.sh <<'END'
#!/bin/sh
# Exits 0 if every price on the menu is from 1 to 20, and 1 if one isn't.
awk '$2 < 1 || $2 > 20 { exit 1 }' menu.txt
END
chmod +x check.sh
printf 'Soup 4\nSalad 5\n' > menu.txt
save "Start the menu"
printf 'Soup 4\nSalad 5\nBread 2\n' > menu.txt
save "Add bread"
git tag v1.0
printf 'Ada\n' > staff.txt
save "Add the staff list"
printf 'Soup 4\nSalad 5\nBread 2\nPie 3\n' > menu.txt
save "Add pie"
printf 'Soup 4\nSalad 5\nBread 2\nPie 3\nCake 40\n' > menu.txt
save "Add cake"
printf 'Ada\nGrace\n' > staff.txt
git config user.name "Grace Hopper"
save "Hire Grace"
printf 'Soup 4\nSalad 6\nBread 2\nPie 3\nCake 40\n' > menu.txt
save "Charge more for salad"
git config user.name "Ada Lovelace"
printf 'Soup 4\nSalad 6\nBread 2\nPie 3\nCake 40\nTea 1\n' > menu.txt
save "Add tea"
```

```console
$ cd cafe
$ git log --oneline
3c86a76 Add tea
c2454a1 Charge more for salad
412ee24 Hire Grace
e01f6f4 Add cake
dc46368 Add pie
c0c57c3 Add the staff list
3f19ce8 Add bread
ef17abc Start the menu
$ git log --oneline -- staff.txt
412ee24 Hire Grace
c0c57c3 Add the staff list
$ git log --oneline -S Cake
e01f6f4 Add cake
$ git log -p -1 --format='%h %s' -- staff.txt
412ee24 Hire Grace

diff --git a/staff.txt b/staff.txt
index a341e20..1922815 100644
--- a/staff.txt
+++ b/staff.txt
@@ -1 +1,2 @@
 Ada
+Grace
```

`git blame` gives each line of a file the commit, author, and date that last changed it.
`^` marks a line that has been there since the root commit. `git grep` searches the
files in the working tree, or in any commit you name, and skips everything that Git
doesn't track, such as `node_modules/`:

```console
$ git blame menu.txt
^ef17abc (Ada Lovelace 2026-03-01 09:02:00 +0000 1) Soup 4
c2454a14 (Grace Hopper 2026-03-01 09:08:00 +0000 2) Salad 6
3f19ce8c (Ada Lovelace 2026-03-01 09:03:00 +0000 3) Bread 2
dc463680 (Ada Lovelace 2026-03-01 09:05:00 +0000 4) Pie 3
e01f6f49 (Ada Lovelace 2026-03-01 09:06:00 +0000 5) Cake 40
3c86a764 (Ada Lovelace 2026-03-01 09:09:00 +0000 6) Tea 1
$ git grep -n Salad
menu.txt:2:Salad 6
$ git grep -n Salad v1.0
v1.0:menu.txt:2:Salad 5
```

When a test passes at an old commit and fails now, `git bisect` finds the commit that
broke it by halving: it checks out a commit in the middle, you say whether it is good or
bad, and it halves again, so a thousand commits take about ten tries. `git bisect run`
does the saying for you: it runs a command at each step, and reads exit code 0 as good,
125 as "can't test this one", and anything else from 1 to 127 as bad. It prints a few
lines for each step, so below its output goes to `/dev/null`, and the answer comes from
`refs/bisect/bad`, which bisect keeps pointed at the first bad commit it knows of.
`git bisect reset` puts you back where you started:

```console
$ ./check.sh && echo good || echo bad
bad
$ git bisect start main v1.0
Bisecting: 2 revisions left to test after this (roughly 2 steps)
[e01f6f49131da4470b7c247ab87f940b435285f7] Add cake
$ git bisect run ./check.sh > /dev/null
$ git log -1 --oneline refs/bisect/bad
e01f6f4 Add cake
$ git bisect reset
Previous HEAD position was dc46368 Add pie
Switched to branch 'main'
```

---

## 17.9 Ignoring files, tags, and settings

Building a program leaves files next to the sources, and none of them belong in the
history: they can be rebuilt, they change with every build, and some are large. Some
files must never be committed at all, such as a `.env` file of passwords. A file called
`.gitignore` lists patterns of paths for Git to leave out. The sessions in this section
start in a folder with a source file and the build output for each of the nine
languages:

```sh
# A project with the files that building it in each of the nine languages leaves
# behind, next to the sources.
mkdir -p app/target/debug app/node_modules/left-pad app/__pycache__ app/build app/.dart_tool
cd app
touch main.c main.cpp main.o main main.go main.rs target/debug/main
touch main.py __pycache__/main.cpython-313.pyc
touch main.js main.ts main.js.map node_modules/left-pad/index.js
touch main.dart .dart_tool/package_config.json main.kt build/main.jar
touch .env notes.txt
git init -q
```

```console
$ cd app
$ git status --short --untracked-files=all
?? .dart_tool/package_config.json
?? .env
?? __pycache__/main.cpython-313.pyc
?? build/main.jar
?? main
?? main.c
?? main.cpp
?? main.dart
?? main.go
?? main.js
?? main.js.map
?? main.kt
?? main.o
?? main.py
?? main.rs
?? main.ts
?? node_modules/left-pad/index.js
?? notes.txt
?? target/debug/main
```

A pattern without a `/` matches a name in any folder; a pattern ending in `/` matches
only folders, and everything in them; a pattern starting with `/` matches only at the
top, so `/main` ignores the compiled program and not a folder called `main` deeper down.
`#` starts a comment, and `!` before a pattern brings back something that an earlier
pattern ignored. `git check-ignore -v` says which line ignores a path:

```console
$ cat > .gitignore <<'END'
> # Build output
> *.o
> /main
> target/
> __pycache__/
> node_modules/
> *.js.map
> .dart_tool/
> build/
> # Secrets
> .env
> END
$ git status --short --untracked-files=all
?? .gitignore
?? main.c
?? main.cpp
?? main.dart
?? main.go
?? main.js
?? main.kt
?? main.py
?? main.rs
?? main.ts
?? notes.txt
$ git check-ignore -v main.o .env build/main.jar
.gitignore:2:*.o	main.o
.gitignore:11:.env	.env
.gitignore:9:build/	build/main.jar
```

| Language | Build output to ignore | Commit, even though a tool writes it |
| --- | --- | --- |
| C, C++ | `*.o`, the program, CMake's `build/` | |
| Rust | `target/` | `Cargo.lock` |
| Go | The program | `go.sum` |
| Python | `__pycache__/`, `.venv/`, `dist/` | `uv.lock` |
| JavaScript, TypeScript | `node_modules/`, `dist/`, `*.js.map` | `package-lock.json` |
| Dart | `.dart_tool/`, `build/` | `pubspec.lock`, for a program |
| Kotlin | `build/`, `.gradle/` | `gradlew` and `gradle/wrapper/` |

The files on the right are **lock files** and the Gradle wrapper. They record the exact
versions of a program's dependencies, or of its build tool, so that everyone builds the
same thing, and chapter 18 is about them. GitHub keeps a collection of `.gitignore`
files for many languages and tools, at github.com/github/gitignore.

A **tag** is a name for a commit that, unlike a branch, doesn't move. An annotated tag,
made with `-a`, is an object of its own with a message, a tagger, and a date, and is the
usual way to mark a release. `git describe` names a commit by the newest tag before it:
`v1.0-1-ga794033` is one commit after `v1.0`, at commit `a794033`.

```console
$ git add .
$ git commit -q -m "Start the app"
$ git tag -a v1.0 -m "First release"
$ echo "more notes" >> notes.txt
$ git commit -q -a -m "Add notes"
$ git describe
v1.0-1-ga794033
$ git tag -n
v1.0            First release
```

Settings come from three files, each overriding the one before: the system's, your own
`~/.gitconfig` (with `--global`), and the repository's `.git/config` (with no option).
`git config --list --show-origin` lists every setting and the file it came from.

Windows ends lines with a carriage return and a line feed, and Linux and macOS with a
line feed alone. A file called `.gitattributes` with the line `* text=auto` makes Git
store every text file with line feeds and give Windows users carriage returns in their
working tree, so that a file doesn't show every line changed after a Windows user edits
it. The same file marks files as binary (`*.png binary`), so that Git never tries to
merge them or show their diffs.

---

## 17.10 Why Git looks the way it does

- **Written for the Linux kernel.** Linux kept its history in BitKeeper, a commercial
  system, until 2005, when the free license that kernel developers used was withdrawn.
  Linus Torvalds wrote Git in a few weeks that April, and Junio Hamano has maintained it
  since that July. The goals came from the kernel: thousands of contributors, patches
  arriving by email, merges every day, and no central server everyone had to trust. So
  merging had to be fast and routine, every copy had to be complete, and the hashes had
  to prove that nothing had been tampered with.
- **Snapshots, not changes.** CVS and Subversion, the systems most projects used before
  Git, number the revisions of each file, or of the whole repository on one central
  server, and you need that server to commit. Git stores whole snapshots, named by their
  contents, and works out the changes when you ask for them. That is why branching is
  cheap (a branch is a 41-byte file) and why renames are guessed from similar contents
  rather than recorded.
- **Plumbing and porcelain.** Git was first a set of low-level commands, which it calls
  plumbing, such as `hash-object`, `cat-file`, and `update-ref`, and the friendly
  commands, the porcelain, were written on top of them, at first as shell scripts. The
  plumbing's output is promised to stay the same, so scripts use it; the porcelain's
  messages change from version to version.
- **Commands that grew.** `git checkout` once did everything to do with the working
  tree: switching branches, making them, and restoring files. That made
  `git checkout NAME` mean very different things depending on whether NAME was a branch
  or a file, so Git 2.23 (2019) split it into `git switch` and `git restore`. `checkout`
  still works, and older documentation and answers online use it.
- **The staging area.** Most version control systems commit every changed file. Git
  makes you choose, which suits the kernel's way of working, where each commit is one
  reviewable change, at the cost of one more idea to learn.
- **Defaults that changed.** The first branch was called `master` until Git 2.28 (2020)
  added `init.defaultBranch`, and GitHub and most hosts now use `main`. Git 2.34 (2021)
  made "ort" the default merge algorithm, which is much faster on large repositories
  than "recursive", the one it replaced.
- **Others.** Mercurial began in the same weeks of 2005, for the same reason, and Meta's
  Sapling grew out of it. Fossil, by the author of
  SQLite, keeps a project's tickets and wiki in the repository too. Jujutsu is a newer
  tool that can work on a Git repository: it has no staging area, and every edit to the
  working tree is part of a commit.

---

## 17.11 Pitfalls

Checking out a commit instead of a branch puts you in **detached HEAD** state. Commits
made there belong to no branch, and switching away leaves them behind, reachable only
from the reflog:

```console
$ git init -q shop
$ cd shop
$ printf 'Soup 4\n' > menu.txt
$ git add menu.txt
$ git commit -q -m "Start the menu"
$ printf 'Soup 4\nSalad 5\n' > menu.txt
$ git commit -q -a -m "Add salad"
$ git checkout HEAD~1
Note: switching to 'HEAD~1'.

You are in 'detached HEAD' state. You can look around, make experimental
changes and commit them, and you can discard any commits you make in this
state without impacting any branches by switching back to a branch.

If you want to create a new branch to retain commits you create, you may
do so (now or later) by using -c with the switch command. Example:

  git switch -c <new-branch-name>

Or undo this operation with:

  git switch -

Turn off this advice by setting config variable advice.detachedHead to false

HEAD is now at d0b3fc8 Start the menu
$ printf 'Soup 4\nBread 2\n' > menu.txt
$ git commit -q -a -m "Add bread"
$ git log --oneline --graph --all
* 90fe440 Add bread
| * eb2a7d9 Add salad
|/
* d0b3fc8 Start the menu
$ git switch main
Warning: you are leaving 1 commit behind, not connected to
any of your branches:

  90fe440 Add bread

If you want to keep it by creating a new branch, this may be a good time
to do so with:

 git branch <new-branch-name> 90fe440

Switched to branch 'main'
```

The first push of a new branch needs `-u`, to say where it goes:

```console
$ git init -q --bare ../shop.git
$ git remote add origin ../shop.git
$ git push
fatal: The current branch main has no upstream branch.
To push the current branch and set the remote as upstream, use

    git push --set-upstream origin main

To have this happen automatically for branches without a tracking
upstream, see 'push.autoSetupRemote' in 'git help config'.
```

`.gitignore` only affects files that Git doesn't already track. A secret, once
committed, stays in the history after you stop tracking it, and anyone with a clone can
read it:

```console
$ printf 'password=hunter2\n' > secrets.env
$ git add secrets.env
$ git commit -q -m "Add settings"
$ echo secrets.env > .gitignore
$ git status --short
?? .gitignore
$ git rm -q --cached secrets.env
$ git add .gitignore
$ git commit -q -m "Stop tracking the settings"
$ git status --short
$ git log --oneline --all -- secrets.env
b06d890 Stop tracking the settings
5a8c14e Add settings
$ git show HEAD~1:secrets.env
password=hunter2
```

Git tracks files, not folders, so an empty folder doesn't exist as far as Git is
concerned. The usual trick is an empty file in it, often called `.gitkeep`:

```console
$ mkdir photos
$ git status --short
$ touch photos/.gitkeep
$ git status --short
?? photos/
```

| Mistake | What happens | Instead |
| --- | --- | --- |
| Committing a password, key, or `.env` file | It is in every clone of the history, even after you delete it | Change the password at once; ignore the file before its first commit |
| `git add .` in a folder of build output | `node_modules/`, `target/`, or the program goes into the commit | Write `.gitignore` first, and read `git status` before committing |
| `git reset --hard` or `git restore` with work you didn't commit | The uncommitted changes are gone; the reflog keeps only commits | `git stash` first, or commit and amend later |
| Rewriting commits that others have, then `git push --force` | Their work is based on commits that no longer exist | `git revert`; force only your own branches, with `--force-with-lease` |
| Committing in detached HEAD state | The commits belong to no branch | `git switch -c NAME` before committing, or right after |
| `git pull` when both sides have new commits | Git stops and asks how to combine them; with `pull.rebase false`, a merge commit for every pull | `git pull --rebase`, or set `pull.rebase true` |
| Adding a file to `.gitignore` after committing it | Git keeps tracking it | `git rm --cached FILE`, and commit |
| Large binary files, such as videos or datasets | Every version stays in every clone forever | Git LFS, or keep them out of the repository |
| Renaming `File.txt` to `file.txt` on macOS or Windows | The file system thinks nothing changed | `git mv File.txt file.txt` |
| One commit for a day of unrelated changes | It can't be reviewed, reverted, or bisected one change at a time | Small commits; `git add -p` to choose hunks |
| A merge conflict resolved by deleting one side | The other side's change is silently lost | Read both sides and the base (`zdiff3`), and run the tests |

---

## 17.12 Idiom check

**Working:**

```sh
git checkout -b fix
git add .
git commit -m "fixed stuff"
git checkout -- menu.txt
git pull
git push --force
```

**Idiomatic:**

```sh
git switch -c fix-soup-price
git add -p                       # choose the hunks that belong in this commit
git commit -m "Fix the price of soup"
git restore menu.txt
git pull --rebase
git push --force-with-lease
```

- **`switch` and `restore`.** They say what they do, where `checkout` does several jobs.
- **Name branches after the work,** such as `fix-soup-price`, so that a list of branches
  says what is in progress.
- **`git add -p`** shows each change and asks whether it goes in, which is also a last
  look at the diff before it becomes history.
- **Messages that say what and why**, with a subject in the imperative. "fixed stuff"
  tells the person bisecting in two years nothing.
- **`--rebase` and `--force-with-lease`** keep the history straight and refuse to throw
  away commits you haven't seen.
- **Commit often, and tidy before sharing.** Small commits and `--fixup` while you work,
  then `git rebase -i --autosquash` before you push, so that what others review is a
  short list of finished changes.

---

## 17.13 Exercises

| # | Exercise | Practises |
| --- | --- | --- |
| 1 | [Merge a branch](exercises/01-merge-a-branch/) | Switching branches, fast-forward and three-way merges, and settling conflicts from a script |
| 2 | [Move commits to a branch](exercises/02-move-commits/) | Branches as labels, `git branch --force`, stashing with the index, and `rebase --autosquash` |
| 3 | [Find the commit that broke it](exercises/03-find-the-bug/) | `git bisect run`, skipping untestable commits, and reading refs |

Each exercise is a bash script of Git commands, `git/main.sh`, which the checker runs in
a throwaway repository that each test's setup makes. The checker then compares what the
repository looks like afterward. It needs `git` and `bash`, and runs on Linux, macOS,
and Windows with Git Bash or WSL:

```sh
python3 tools/check.py 17                # your answers
python3 tools/check.py 17 --solutions    # the reference answers, and the examples
```

**Stretch:** put your own answers to the earlier chapters' exercises in a repository,
one commit per exercise, with a `.gitignore` that keeps out every build output from
the table in 17.9. Push it to a new repository on GitHub, change one solution on a
branch, and open a pull request to yourself to see what a reviewer sees.

---

Previous: [Chapter 16: Concurrency and parallelism](../16-concurrency-and-parallelism/) ·
Next: Chapter 18: Modules, packages, builds (coming soon)
