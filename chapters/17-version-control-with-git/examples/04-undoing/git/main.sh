cd cafe
git log --oneline
printf 'Soup 40\nSalad 5\nPie 3\n' > menu.txt
git status --short
git restore menu.txt
git status --short
printf 'Soup 4\nSalad 5\nPie 3\nCake 4\n' > menu.txt
git add menu.txt
git status --short
git restore --staged menu.txt
git status --short
# ---
git commit -q -a -m "Add cak"
git log --oneline -1
git commit -q --amend -m "Add cake"
git log --oneline
# ---
git reset --soft HEAD~1
git status --short
git log --oneline -1
git reset HEAD~1
git status --short
git log --oneline -1
# ---
git reset --hard HEAD~1
git status --short
cat menu.txt
git log --oneline
git reflog
git reset --hard HEAD@{3}
git log --oneline
# ---
git revert --no-edit HEAD~2
git log --oneline
ls
