# Exercise 17.1: Merge a branch

Write a script of Git commands that merges a branch called `feature` into `main`, and
handles a conflict without stopping to ask.

Each test starts with a repository that has two branches, `main` and `feature`, and no
uncommitted changes. The test's `.in` file is the bash script that made it. Here is the
first one, `tests/01-sample.in`. Its `save` function commits everything, a minute after
the commit before, and `tick` moves the clock that Git reads:

```sh
git init -q
save() { tick; git add --all; git commit -q -m "$1"; }

printf 'Soup\nSalad\n' > menu.txt
save "Start the menu"
printf '# Recipes\n' > recipes.md
save "Add a recipe file"

git switch -q -c feature
printf 'Soup\nSalad\nPie\n' > menu.txt
save "Add pie"
printf 'Soup 4\nSalad 5\nPie 3\n' > prices.txt
save "Add prices"

git switch -q main
printf '# Recipes\n\nSoup: boil water, add vegetables.\n' > recipes.md
save "Write the soup recipe"
```

Your script, `git/main.sh`, runs next, in the repository's top folder. Then
`tests/report.sh` prints what the repository looks like, and that is what the test
compares. For the sample, it prints:

```text
on branch: main
branches: main
history of main:
*   Merge branch 'feature'
|\  
| * Add prices
| * Add pie
* | Write the soup recipe
|/  
* Add a recipe file
* Start the menu
uncommitted changes:
--- menu.txt
Soup
Salad
Pie
--- prices.txt
Soup 4
Salad 5
Pie 3
--- recipes.md
# Recipes

Soup: boil water, add vegetables.
```

The rules are:

- **Bring every commit on `feature` into `main`.** If `main` hasn't moved since
  `feature` started from it, `main` fast-forwards to `feature`, with no merge commit.
  Otherwise there is a merge commit, with Git's usual message, `Merge branch 'feature'`.
  If `main` already has every commit on `feature`, it stays as it is.
- **Where both branches changed the same lines, keep the lines from `feature`.** Every
  other change, from either branch, is kept, including the changes that `main` made to a
  conflicted file outside the lines that conflict.
- **Afterward,** delete the `feature` branch, and leave `main` checked out with nothing
  uncommitted. Either branch can be checked out when your script starts.

Your script can print anything it likes: only the report is compared. No editor opens in
the tests, so a command that would open one fails, as it would in any script.

```sh
python3 tools/check.py 17
```

**Things to notice:**

- **Fast-forward.** When `main` is an ancestor of `feature`, merging needs no new
  commit: Git moves the `main` label up to where `feature` is, and the history stays a
  straight line. `--no-ff` makes a merge commit anyway, which some teams prefer because
  it keeps a record of the branch, and test 2 fails with it.
- **Which side is "theirs".** During a merge, "ours" is the branch you are on and
  "theirs" is the one being merged in. During a rebase it is the other way around,
  because a rebase replays your commits on top of the other branch.
- **Two ways to take their side.** `git checkout --theirs FILE` takes their whole
  version of a conflicted file, and throws away every change that `main` made to it,
  conflicting or not. The merge option `-X theirs` (`--strategy-option=theirs`) takes
  their side only of the lines that conflict, and test 4 tells the two apart. Neither is
  right in general: a real conflict needs someone to read both sides.
- **Conflicts that `-X theirs` can't settle.** If one branch changes a file and the
  other deletes it, there are no lines to choose between, and the merge stops with
  `CONFLICT (modify/delete)` whatever option you give. The tests leave that case out.

**Stretch:** change the rule so that a conflict keeps both sides, `main`'s lines first.
Git can do that for chosen files with `merge=union` in `.gitattributes`. Try it, and
work out why it suits a changelog and not a program.
