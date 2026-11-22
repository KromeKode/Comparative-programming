git init -q notes
cd notes
echo hello > greeting.txt
git hash-object greeting.txt
echo hello | git hash-object --stdin
printf 'blob 6\0hello\n' | sha1sum
git add greeting.txt
find .git/objects -type f
git cat-file -t ce01362
git cat-file -p ce01362
# ---
git commit -q -m "Say hello"
git cat-file -p HEAD
git cat-file -p 'HEAD^{tree}'
cat .git/HEAD
cat .git/refs/heads/main
# ---
cp greeting.txt copy.txt
mkdir docs
echo "Notes about notes." > docs/about.txt
git add .
git commit -q -m "Add a copy and a folder"
git cat-file -p 'HEAD^{tree}'
git cat-file -p HEAD
find .git/objects -type f | sort
