#!/usr/bin/env bash

TERM="filesystem"
SEARCH_DIR="/usr/include/boost"
echo "Searching in $SEARCH_DIR for $TERM"

TRACE=/tmp/fsrc/trace.txt
mkdir -p /tmp/fsrc
[[ -f "$TRACE" ]] && rm "$TRACE"


function append {
    echo "$1" | tee -a "$TRACE"
}

append
append "fsrc"
append "strace -c ./fsrc --no-git -t \"$TERM\" -d \"$SEARCH_DIR\""
strace -c ./fsrc --no-git -t "$TERM" -d "$SEARCH_DIR" 2>> "$TRACE" | tail -n 1
append

append
append "rg"
append "strace -c rg --stats -e \"$TERM\" \"$SEARCH_DIR\""
strace -c rg --stats -e "$TERM" "$SEARCH_DIR" 2>> "$TRACE" | tail -n 3
append

xdg-open "$TRACE"

