# Prints what your script printed, then what the repository looks like.
echo "printed:"
cat "$1"
echo "on branch: $(git branch --show-current)"
if git bisect log > /dev/null 2>&1; then
    echo "a bisect is still going"
fi
echo "uncommitted changes:"
git status --porcelain
