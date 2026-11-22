cd cafe
git log --oneline --graph --all
git switch soups
git rebase main
git log --oneline --graph --all
# ---
git switch main
git merge soups
git branch -d soups
# ---
git switch -c winter
printf 'Open 10 to 4\n' > hours.txt
git commit -q -a -m "Open later in winter"
printf 'Soup 5\nSalad 5\n' > menu.txt
git commit -q -a -m "Charge more for soup"
git switch main
git cherry-pick winter
git log --oneline --graph --all
# ---
printf 'Ada\nGrace\n' > staff.txt
git status --short
git stash
git status --short
git stash list
git stash pop
git status --short
# ---
git restore staff.txt
printf 'Open 9 to 6\n' > hours.txt
git commit -q -a -m "Open an hour later"
printf 'Grace\n' > cook.txt
git add cook.txt
git commit -q -m "Name the cook"
printf 'Open 9 to 7\n' > hours.txt
git commit -q -a --fixup HEAD~1
git log --oneline -4
GIT_SEQUENCE_EDITOR="sed -n 's/ # / /; /^[^#]/p'" git rebase -i --autosquash HEAD~3
git log --oneline -3
