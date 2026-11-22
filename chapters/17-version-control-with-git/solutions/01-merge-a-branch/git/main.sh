# Merge feature into main, taking feature's side of any conflicting lines.
git switch --quiet main
git merge --quiet --strategy-option=theirs feature
git branch --quiet --delete feature
