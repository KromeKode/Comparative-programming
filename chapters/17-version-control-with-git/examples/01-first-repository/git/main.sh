git init notes
cd notes
printf 'milk\neggs\n' > shopping.txt
git status
# ---
git add shopping.txt
git status
git commit -m "Start a shopping list"
# ---
printf 'bread\n' >> shopping.txt
git status --short
git diff
git add shopping.txt
git status --short
git diff --staged
git commit -m "Add bread"
# ---
git log
git log --oneline
