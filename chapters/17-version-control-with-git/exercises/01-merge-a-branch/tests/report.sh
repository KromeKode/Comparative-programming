# Prints what the repository looks like after your script has run.
echo "on branch: $(git branch --show-current)"
echo "branches: $(git branch --format='%(refname:short)' | paste -sd ' ' -)"
echo "history of main:"
git log --graph --format=%s main
echo "uncommitted changes:"
git status --porcelain
for file in $(git ls-files); do
    echo "--- $file"
    cat "$file"
done
