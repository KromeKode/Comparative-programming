# Exercise 17.2: Move commits to a branch

You have been committing straight to `main`, and those commits should have been on a
branch of their own, so that they can be reviewed before they join `main`. Some of them
are `fixup!` commits, small corrections to an earlier commit that you made with
`git commit --fixup`. Write a script of Git commands that moves the commits to a new
branch, folds in the corrections, and puts `main` back where it was.

Each test starts with a repository on `main`, with a tag called `published` on the last
commit that other people have. The test's `.in` file is the bash script that made it.
Here is the first one, `tests/01-sample.in`:

```sh
git init -q
save() { tick; git add --all; git commit -q -m "$1"; }
printf 'Soup\nSalad\n' > menu.txt
save "Start the menu"
printf 'Ada\n' > staff.txt
save "Add the staff list"
git tag published

printf 'Soup: boil water\n' > soup.txt
save "Write the soup recipe"
printf 'Pie: bake it\n' > pie.txt
save "Write the pie recipe"
printf 'Soup: boil water, add leeks\n' > soup.txt
save "fixup! Write the soup recipe"
```

Your script, `git/main.sh`, runs next, in the repository's top folder. Then
`tests/report.sh` prints what the repository looks like, and that is what the test
compares. For each commit on `topic` after `published`, it prints the files that the
commit changed, with what they held in that commit, the lines joined with `|`. For the
sample, it prints:

```text
on branch: topic
branches: main topic
main is at published
commits on topic after published, with the files each one changed:
* Write the soup recipe
    soup.txt: Soup: boil water, add leeks
* Write the pie recipe
    pie.txt: Pie: bake it
uncommitted changes:
stash entries:
```

The rules are:

- **Make a branch called `topic`** that has every commit on `main` after `published`, in
  the same order, except that each commit whose message is `fixup! SUBJECT` is folded
  into the earlier commit whose subject is SUBJECT, which keeps its own message. A fixup
  of a fixup goes into the same commit. A fixup of a commit that is already published
  has nothing on `topic` to fold into, so it stays a commit of its own.
- **Move `main` back to `published`.**
- **End with `topic` checked out,** and with the uncommitted changes that were there at
  the start just as they were: staged changes still staged, changes that weren't staged
  still not staged, and files that Git doesn't track still there.
- **Leave any stash entries alone.** Some tests have stashed work from before.

Your script can print anything it likes: only the report is compared. No editor opens in
the tests, so a command that would open one fails, as it would in any script.

```sh
python3 tools/check.py 17
```

**Things to notice:**

- **A branch is a label.** Making `topic` copies nothing: it is a new label on the
  commit that `main` is on. Moving `main` back is moving a label too, and the commits
  are still there, because `topic` points at them. `git branch --force main published`
  moves a branch that isn't checked out, and the working tree doesn't change.
- **The rebase list.** An interactive rebase opens an editor with a list of the commits
  it will replay, and `--autosquash` has already moved each `fixup!` commit under the
  one it fixes and marked it `fixup`. In a script, `GIT_SEQUENCE_EDITOR=:` makes the
  "editor" a command that does nothing, so the list is used as it is. In Git 2.43, which
  made the tests' outputs, `--autosquash` does nothing without `--interactive`. Git 2.44
  changed that.
- **A rebase needs a clean working tree.** It refuses to start with
  `error: cannot rebase: You have unstaged changes.`, so the changes have to go
  somewhere while it runs. `git stash` keeps them in a commit of their own, but
  `git stash pop` without `--index` puts staged changes back as unstaged ones, and so
  does `git rebase --autostash`. Tests 6, 8, and 9 fail with either.
- **An empty stash.** With nothing to stash, `git stash` prints
  `No local changes to save` and makes no entry. A script that then runs `git stash pop`
  takes the newest entry that was already there, which was someone's half-finished work.
  Test 7 fails that way.

**Stretch:** some of the moved commits now conflict with each other once the fixups are
moved up next to the commits they fix. Make a test where a fixup changes a line that a
later commit changes too, run your script on it, and work out what state the rebase
leaves the repository in, and which commands finish or abandon it.
