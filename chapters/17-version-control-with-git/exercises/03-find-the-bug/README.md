# Exercise 17.3: Find the commit that broke it

Somewhere between the release tagged `v1.0` and the latest commit on `main`, a test
started failing. Write a script of Git commands that finds the first commit where it
fails, without checking every commit by hand, and prints that commit's subject.

Each test starts with a repository whose test is `check.sh`. It exits with 0 when a
commit is good, 1 when it is bad, and 125 when a commit can't be tested at all. The
test's `.in` file is the bash script that made the repository. Here is the first one,
`tests/01-sample.in`:

```sh
git init -q
save() { tick; git add --all; git commit -q -m "$1"; }

# check.sh is the test. It passes when every dish on the menu has a price from
# 1 to 20, fails when one doesn't, and exits 125 when there is no menu to check.
cat > check.sh <<'END'
#!/bin/sh
[ -d menu ] || exit 125
for dish in menu/*; do
    price=$(cat "$dish")
    [ "$price" -ge 1 ] 2> /dev/null && [ "$price" -le 20 ] || exit 1
done
END
chmod +x check.sh
mkdir menu
echo 4 > menu/soup
save "Open the shop"

# add puts a dish on the menu, in a file of its own that holds its price.
add() {
    echo "$2" > "menu/$1"
    save "Add $1"
}

add salad 5
add bread 2
git tag v1.0
add pie 3
add tea 1
echo 40 > menu/soup
save "Charge more for soup"
add cake 4
add coffee 2
add juice 3
```

Your script, `git/main.sh`, runs next, in the repository's top folder. Then
`tests/report.sh` prints what your script printed and what the repository looks like
afterward, and that is what the test compares. For the sample, it prints:

```text
printed:
Charge more for soup
on branch: main
uncommitted changes:
```

The rules are:

- **`v1.0` is good and `main` is bad,** and every commit after the first bad one is bad
  too. Find the first bad commit, the one whose parents are all good.
- **A commit where `check.sh` exits with 125 can't be tested.** It is neither good nor
  bad, and the tests never put one right before the first bad commit, so the answer is
  always a single commit.
- **Print one line,** the first bad commit's subject, and nothing else. Git's own output
  has to go to `/dev/null`.
- **Leave the repository as you found it,** with the same branch checked out (it isn't
  always `main`) and no bisect still going.

```sh
python3 tools/check.py 17
```

**Things to notice:**

- **Halving.** `git bisect` checks out the commit halfway between the newest good commit
  and the oldest bad one, and each answer throws away half of what's left. Test 7 has
  300 commits after `v1.0`, and `git bisect run` needs 9 tries, where checking every
  commit in turn would need up to 300.
- **`git bisect run`** runs a command on each commit it checks out and reads its exit
  status. 125 is set aside for "skip this one", so a test that can't build a commit can
  say so instead of calling it bad. If `check.sh` exited with 1 there instead, test 5
  would blame the first commit that took the menu away.
- **Where the answer is.** When the run ends, `refs/bisect/bad` points at the first bad
  commit, and Git has printed it, with its hash, message, and changed files. Read the
  ref before `git bisect reset`, which deletes it and checks out the branch you started
  on.
- **Merges.** Bisecting follows every parent, so the first bad commit can be on a branch
  that was merged in later, as in test 4. `git bisect start --first-parent` looks only
  at the commits on `main` itself, and would blame the merge instead.

**Stretch:** remove the rule that keeps untestable commits away from the answer, and put
one right before the first bad commit. What does `git bisect run` say then, and what
should your script print?
