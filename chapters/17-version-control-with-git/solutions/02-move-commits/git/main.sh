# Move the unpublished commits on main to a new branch, topic, and fold in the
# fixup! commits, keeping any uncommitted changes as they were.

# A new branch starts where main is, and switching to it keeps the uncommitted
# changes. Then main can move back to published without touching them.
git switch --quiet --create topic
git branch --force main published

# A rebase needs a clean working tree. git stash makes no entry when there is
# nothing to stash, so check first: popping would take an older entry instead.
if git diff --quiet && git diff --cached --quiet; then
    stashed=false
else
    git stash push --quiet
    stashed=true
fi

# --autosquash moves each fixup! commit to just after the commit it fixes and
# folds it in. It works only in an interactive rebase, and ":" as the editor
# accepts the list of steps as it is.
GIT_SEQUENCE_EDITOR=: git rebase --quiet --interactive --autosquash published

# --index puts staged changes back in the index, not just in the working tree.
if $stashed; then
    git stash pop --quiet --index
fi
