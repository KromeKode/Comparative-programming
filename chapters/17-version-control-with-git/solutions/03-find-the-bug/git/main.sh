# Find the first commit after v1.0 where check.sh fails, and print its subject.

# main is bad and v1.0 is good. git bisect checks out a commit halfway between,
# and run tests each one with check.sh until a single commit is left: 0 means
# good, 125 means "can't tell, try a neighbor", and anything else means bad.
git bisect start main v1.0 > /dev/null
git bisect run ./check.sh > /dev/null

# The answer is the commit that refs/bisect/bad points at.
git log -1 --format=%s refs/bisect/bad

# Leave the repository as it was found, on the branch it started on.
git bisect reset > /dev/null 2>&1
