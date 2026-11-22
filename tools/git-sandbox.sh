#!/usr/bin/env bash
# Run a chapter 17 Git script in a fresh, throwaway directory.
#
#   bash tools/git-sandbox.sh FOLDER SCRIPT TESTS < SETUP
#
# SETUP (a test's .in file) is a bash script that builds the starting repository.
# It runs in an empty folder, which is where SCRIPT then runs too. FOLDER says
# what kind of program SCRIPT is:
#
#   examples    printed as a terminal session: each command after "$ ", then what
#               it printed, stdout and stderr together
#   solutions,  run as they are. Then TESTS/report.sh, if there is one, prints
#   exercises   what the repository looks like, and what SCRIPT printed if it
#               matters: its output is in the file named by report.sh's $1.
#
# Git's settings come from this script alone, never from your own ~/.gitconfig.
# Every commit has the same author, and a clock that starts at the same time and
# moves on a minute for each command, so the commit hashes come out the same on
# every machine. The folder shows up as /home/ada in the output.
set -u

folder=$1
script=$(realpath "$2")
tests=$(realpath "$3")
# Everything happens in $work, which stands in for a home folder. $own holds this
# script's files: Git's settings, and the setup's output. They stay out of $work
# so that a `git add .` there can't pick them up. pwd -P gives the real path,
# which is what Git prints (on macOS /tmp is a link).
work=$(cd "$(mktemp -d)" && pwd -P)
own=$(cd "$(mktemp -d)" && pwd -P)
trap 'rm -rf "$work" "$own"' EXIT

export HOME=$work LANG=C.UTF-8 LC_ALL=C.UTF-8 TZ=UTC
export GIT_CONFIG_NOSYSTEM=1 GIT_CONFIG_GLOBAL=$own/.gitconfig
# A script can't answer an editor, so a command that would open one fails instead.
export GIT_EDITOR=false
unset GIT_DIR GIT_WORK_TREE GIT_INDEX_FILE GIT_PAGER PAGER
git config --global user.name "Ada Lovelace"
git config --global user.email ada@example.com
git config --global init.defaultBranch main

# tick moves the clock one minute on, as Git's own test suite does, so that each
# commit in a setup gets its own time. The clock starts at 9:00 on 1 March 2026.
export TICK=1772355600
tick() {
    TICK=$((TICK + 60))
    export GIT_AUTHOR_DATE="@$TICK +0000" GIT_COMMITTER_DATE="@$TICK +0000"
}
export -f tick
tick

# An example starts in the home folder, and an exercise in the repository that
# its setup makes.
if [[ $folder == examples ]]; then here=$work; else here=$work/repo; fi
mkdir -p "$here"
cd "$here" || exit 1
# The setup's last line saves its clock, so that SCRIPT's commits come after the
# setup's.
if ! { cat; printf '\necho "$TICK" > "%s"\n' "$own/tick"; } | bash -e > "$own/setup.log" 2>&1
then
    echo "the test's setup failed:" >&2
    cat "$own/setup.log" >&2
    exit 1
fi
TICK=$(cat "$own/tick")
tick

set_status() { return "$1"; }

session() {
    # Print each command, then run it, a minute after the one before. A
    # heredoc's lines follow its command, after "> ", as a shell shows them.
    # "# ---" separates the parts of a session that the text shows one at a time.
    local line block end last=0
    while IFS= read -r line <&3 || [[ -n $line ]]; do
        case $line in
            '') continue ;;
            '# ---'*) echo '# ---'; continue ;;
            '#'*) continue ;;
        esac
        printf '$ %s\n' "$line"
        block=$line
        if [[ $line =~ \<\<-?\'?([A-Za-z_]+)\'?$ ]]; then
            end=${BASH_REMATCH[1]}
            while IFS= read -r line <&3; do
                printf '> %s\n' "$line"
                block+=$'\n'$line
                [[ $line == "$end" ]] && break
            done
        fi
        tick
        # Hand the last command's exit status on, so that "echo $?" works.
        set_status "$last"
        eval "$block" < /dev/null 2>&1
        last=$?
    done 3< "$1"
    return 0
}

# Show the output as a terminal would: the folders as /home/ada, and a line that
# Git rewrites in place (with a carriage return, as "Rebasing (2/3)" does) as
# only its last version.
tidy() {
    # $'\r' and $'\e' are the characters themselves, which every sed understands.
    sed -e "s#$own/#/home/ada/#g" -e "s#$work#/home/ada#g" \
        -e "s/.*"$'\r'"//" -e "s/"$'\e'"\\[K//g"
}

if [[ $folder == examples ]]; then
    session "$script" | tidy
    exit "${PIPESTATUS[0]}"
fi

# The script's output goes to a file, which report.sh can print (or not) as the
# first of its arguments.
bash "$script" < /dev/null > "$own/printed" 2> >(tidy >&2)
status=$?
tidy < "$own/printed" > "$own/printed.txt"
cd "$here" || exit 1
if [[ -f $tests/report.sh ]]; then
    bash "$tests/report.sh" "$own/printed.txt" < /dev/null 2>&1 | tidy
else
    cat "$own/printed.txt"
fi
exit "$status"
