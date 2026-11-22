git init -q shop
cd shop
printf 'Soup 4\n' > menu.txt
git add menu.txt
git commit -q -m "Start the menu"
printf 'Soup 4\nSalad 5\n' > menu.txt
git commit -q -a -m "Add salad"
git checkout HEAD~1
printf 'Soup 4\nBread 2\n' > menu.txt
git commit -q -a -m "Add bread"
git log --oneline --graph --all
git switch main
# ---
git init -q --bare ../shop.git
git remote add origin ../shop.git
git push
# ---
printf 'password=hunter2\n' > secrets.env
git add secrets.env
git commit -q -m "Add settings"
echo secrets.env > .gitignore
git status --short
git rm -q --cached secrets.env
git add .gitignore
git commit -q -m "Stop tracking the settings"
git status --short
git log --oneline --all -- secrets.env
git show HEAD~1:secrets.env
# ---
mkdir photos
git status --short
touch photos/.gitkeep
git status --short
