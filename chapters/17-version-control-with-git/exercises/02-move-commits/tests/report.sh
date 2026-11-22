# Prints what the repository looks like after your script has run.
echo "on branch: $(git branch --show-current)"
echo "branches: $(git branch --format='%(refname:short)' | paste -sd ' ' -)"
if [ "$(git rev-parse main)" = "$(git rev-parse 'published^{commit}')" ]; then
    echo "main is at published"
else
    echo "main is NOT at published"
fi
echo "commits on topic after published, with the files each one changed:"
for commit in $(git rev-list --reverse published..topic); do
    echo "* $(git log -1 --format=%s "$commit")"
    for file in $(git diff-tree --no-commit-id --name-only -r "$commit"); do
        if git cat-file -e "$commit:$file" 2> /dev/null; then
            echo "    $file: $(git show "$commit:$file" | paste -sd '|' -)"
        else
            echo "    $file: (deleted)"
        fi
    done
done
echo "uncommitted changes:"
git status --porcelain
echo "stash entries:"
git stash list --format=%s
