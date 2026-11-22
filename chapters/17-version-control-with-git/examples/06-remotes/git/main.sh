git init -q --bare cafe.git
git clone cafe.git ada
cd ada
printf 'Soup 4\nSalad 5\n' > menu.txt
git add menu.txt
git commit -q -m "Start the menu"
git push -u origin main
git branch -vv
# ---
cd ..
git clone -q cafe.git grace
cd grace
git config user.name "Grace Hopper"
git config user.email grace@example.com
printf 'Grace\n' > staff.txt
git add staff.txt
git commit -q -m "Add the staff list"
git push -q
# ---
cd ../ada
printf 'Soup 4\nSalad 5\nPie 3\n' > menu.txt
git commit -q -a -m "Add pie"
git push
git fetch
git status
git log --oneline --graph --all
# ---
git pull
git pull --rebase
git log --oneline --graph --all
git push
git log --format='%h %an: %s'
