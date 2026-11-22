git init -q cafe
cd cafe
printf 'Soup 4\nSalad 5\nBread 2\n' > menu.txt
git add menu.txt
git commit -q -m "Start the menu"
git branch pie
git branch
git switch pie
printf 'Pie 3\n' >> menu.txt
git commit -q -a -m "Add pie"
git log --oneline --graph --all
# ---
git switch main
git merge pie
git log --oneline --graph --all
# ---
git switch -c drinks
printf 'Tea 1\nCoffee 2\n' > drinks.txt
git add drinks.txt
git commit -q -m "Add a drinks list"
git switch main
printf 'Soup 4\nSalad 5\nBread 3\nPie 3\n' > menu.txt
git commit -q -a -m "Charge more for bread"
git log --oneline --graph --all
git merge -m "Merge the drinks list" drinks
git log --oneline --graph
git branch -d pie drinks
# ---
git switch -c cheaper
printf 'Soup 3\nSalad 5\nBread 3\nPie 3\n' > menu.txt
git commit -q -a -m "Make soup cheaper"
git switch main
printf 'Soup 5\nSalad 5\nBread 3\nPie 3\n' > menu.txt
git commit -q -a -m "Make soup dearer"
git merge cheaper
git status
cat menu.txt
git checkout --conflict=zdiff3 menu.txt
cat menu.txt
# ---
printf 'Soup 4\nSalad 5\nBread 3\nPie 3\n' > menu.txt
git add menu.txt
git status --short
git commit -q --no-edit
git log --oneline --graph
