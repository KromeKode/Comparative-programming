cd cafe
git log --oneline
git log --oneline -- staff.txt
git log --oneline -S Cake
git log -p -1 --format='%h %s' -- staff.txt
# ---
git blame menu.txt
git grep -n Salad
git grep -n Salad v1.0
# ---
./check.sh && echo good || echo bad
git bisect start main v1.0
git bisect run ./check.sh > /dev/null
git log -1 --oneline refs/bisect/bad
git bisect reset
