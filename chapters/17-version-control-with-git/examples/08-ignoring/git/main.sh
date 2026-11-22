cd app
git status --short --untracked-files=all
# ---
cat > .gitignore <<'END'
# Build output
*.o
/main
target/
__pycache__/
node_modules/
*.js.map
.dart_tool/
build/
# Secrets
.env
END
git status --short --untracked-files=all
git check-ignore -v main.o .env build/main.jar
# ---
git add .
git commit -q -m "Start the app"
git tag -a v1.0 -m "First release"
echo "more notes" >> notes.txt
git commit -q -a -m "Add notes"
git describe
git tag -n
